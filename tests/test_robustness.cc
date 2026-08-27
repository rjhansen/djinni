/* Copyright (c) 2004 - 2026, Robert J. Hansen <rjh@sixdemonbag.org>
 * and Tristan D. Thiede (address currently unknown).
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE. */

// Regression coverage for a batch of correctness/robustness fixes that were
// each found and fixed by hand (manual review plus one-off scratch
// programs), without a test at the time: an out-of-bounds 2-opt index
// picker on undersized worlds, an unsigned-underflow crash on zero-customer
// worlds, uncaught parser exceptions on malformed numeric fields, a
// silently-empty world on a missing input file, Annealer::solve() not
// resetting its convergence state on reuse, Annealer's shared_ptr-era
// copy-aliasing hazard (now closed by deleting Annealer's copy operations
// and giving TravelingSalesmanSolution real move semantics), and
// out-of-range multT/accept values silently producing NaN/inf instead of
// erroring. This file exists so a regression in any of them would actually
// be caught.

#include "djinni.h"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

using edu::uiowa::tippie::djinni::Annealer;
using edu::uiowa::tippie::djinni::Compression;
using edu::uiowa::tippie::djinni::TravelingSalesman;
using edu::uiowa::tippie::djinni::TravelingSalesmanWorld;

// update() must not be reachable from outside the class: it assumes
// _firstswitch/_secondswitch are already populated by generateNeighbor(),
// which isn't true immediately after construction.
template <class T>
concept HasPublicUpdate = requires(T t) { t.update(); };
static_assert(!HasPublicUpdate<TravelingSalesman>);

// Annealer must stay non-copyable (a compiler-generated copy would still
// correlate the original's and the copy's temperature-acceptance RNG
// streams) but must stay movable (e.g. to return one by value).
using TestAnnealer = Annealer<Compression, TravelingSalesman>;
static_assert(!std::is_copy_constructible_v<TestAnnealer>);
static_assert(!std::is_copy_assignable_v<TestAnnealer>);
static_assert(std::is_move_constructible_v<TestAnnealer>);
static_assert(std::is_move_assignable_v<TestAnnealer>);

// TravelingSalesmanSolution needs real move semantics: Annealer relies on
// them to keep its _current/_neighbor swaps O(1).
static_assert(std::is_move_constructible_v<TravelingSalesman>);
static_assert(std::is_move_assignable_v<TravelingSalesman>);

int main(int argc, char **argv) {
  const std::string filename = argc > 1 ? argv[1] : "Dumas-1.set";

  // A world with fewer than 4 customers can't safely pick 2-opt switch
  // points (see TravelingSalesmanSolution::kMinCustomers in routes.h) --
  // constructing a solution from one must fail loudly, not read or write
  // out of bounds.
  {
    auto tiny = TravelingSalesmanWorld::loadFromDumasString(
        "1  0.0  0.0  0.0  0.0  1000.0  0.0\n"
        "2  1.0  1.0  0.0  0.0  1000.0  0.0\n"
        "3  2.0  2.0  0.0  0.0  1000.0  0.0\n"
        "999\n");
    bool threw = false;
    try {
      TravelingSalesman sol(tiny);
    } catch (const std::invalid_argument &) {
      threw = true;
    }
    assert(threw);
  }

  // A world with zero customer rows must not crash while computing travel
  // times (numCustomers - 1 used to underflow as a uint32_t loop bound).
  {
    auto empty = TravelingSalesmanWorld::loadFromDumasString("999\n");
    assert(empty.data().size() == 0);
  }

  // A numeric field with more digits than fit in int/double must raise a
  // catchable error, not an uncaught std::out_of_range that aborts the
  // process.
  {
    std::string huge_id(50, '9');
    bool threw = false;
    try {
      TravelingSalesmanWorld::loadFromDumasString(
          huge_id + "  0.0  0.0  0.0  0.0  1000.0  0.0\n999\n");
    } catch (const std::invalid_argument &) {
      threw = true;
    }
    assert(threw);
  }

  // A nonexistent file must raise a catchable error, not silently produce
  // an empty world.
  {
    bool threw = false;
    try {
      TravelingSalesmanWorld::loadFromDumasFile(
          "/no/such/file/djinni-test-does-not-exist.set");
    } catch (const std::runtime_error &) {
      threw = true;
    }
    assert(threw);
  }

  // multT/accept outside (0.0, 1.0) must be rejected up front instead of
  // silently producing NaN/inf via log() that only surfaces later.
  {
    auto world = TravelingSalesmanWorld::loadFromDumasString(
        "1  0.0  0.0  0.0  0.0  1000.0  0.0\n"
        "2  1.0  1.0  0.0  0.0  1000.0  0.0\n"
        "3  2.0  2.0  0.0  0.0  1000.0  0.0\n"
        "4  3.0  3.0  0.0  0.0  1000.0  0.0\n"
        "999\n");
    auto sol = TravelingSalesman(world);
    auto penalty = Compression(0.06, 0.0, 0.9999);
    for (double bad : {0.0, 1.0, -0.5, 1.5}) {
      bool threw = false;
      try {
        TestAnnealer annealer(penalty, sol, bad, 0.94, 75, 100, 30000);
      } catch (const std::invalid_argument &) {
        threw = true;
      }
      assert(threw);
      threw = false;
      try {
        TestAnnealer annealer(penalty, sol, 0.95, bad, 75, 100, 30000);
      } catch (const std::invalid_argument &) {
        threw = true;
      }
      assert(threw);
    }
  }

  // solve() must reset its convergence state each call. bestIter starts
  // at 0 and increments once per outer iteration (dropping to 1, never
  // below, whenever a new best is found), so in a properly reset run
  // bestIter can exceed iterations by at most 1. Before the fix, reusing
  // an Annealer via setSolutionParameters() left bestIter at whatever the
  // previous run had reached, which could exceed iterations by however far
  // that prior run had already progressed.
  if (std::filesystem::exists(filename)) {
    auto world = TravelingSalesmanWorld::loadFromDumasFile(filename);
    auto initial = TravelingSalesman(world);
    auto penalty = Compression(0.06, 0.0, 0.9999);
    TestAnnealer annealer(penalty, initial, 0.95, 0.94, 75, 100, 30000);
    annealer.solve();
    annealer.setSolutionParameters(filename.c_str());
    annealer.solve();
    assert(annealer.bestIter() <= annealer.iterations() + 1);
  } else {
    std::cerr << "warning: '" << filename
              << "' not found, skipping solve()-reuse check\n";
  }

  return 0;
}
