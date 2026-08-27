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

// Exercises TravelingSalesmanSolution(const char*) directly, and
// Annealer::setSolutionParameters(), which goes through the same
// constructor. This constructor used to call
// WorldType::loadFromDumasFile() with no arguments -- which doesn't
// compile once the constructor is actually instantiated -- but nothing
// else in this codebase ever called it, so the bug went undetected until
// it was instantiated by hand outside the test suite. This test exists so
// a regression here would actually be caught.

#include "djinni.h"

#include <cassert>
#include <filesystem>
#include <iostream>

using edu::uiowa::tippie::djinni::Annealer;
using edu::uiowa::tippie::djinni::Compression;
using edu::uiowa::tippie::djinni::TravelingSalesman;
using edu::uiowa::tippie::djinni::TravelingSalesmanWorld;

int main(int argc, char **argv) {
  const std::string filename = argc > 1 ? argv[1] : "Dumas-1.set";
  if (!std::filesystem::exists(filename)) {
    std::cerr << "Error: couldn't find the file '" << filename << "'.\n";
    return 1;
  }

  // Directly exercise TravelingSalesmanSolution(const char*).
  TravelingSalesman direct(filename.c_str());
  direct.randomize();
  direct.compute();
  assert(direct.getF() > 0.0);

  // Exercise the same constructor indirectly through
  // Annealer::setSolutionParameters(). The Annealer needs a well-formed
  // solution to be constructed at all, so build a throwaway one first and
  // then immediately replace it via setSolutionParameters().
  auto world = TravelingSalesmanWorld::loadFromDumasFile(filename);
  auto placeholder_solution = TravelingSalesman(world);
  auto penalty_function = Compression(0.06, 0.0, 0.9999);
  Annealer annealer(penalty_function, placeholder_solution, 0.95, 0.94, 75, 100,
                    30000);
  annealer.setSolutionParameters(filename.c_str());
  assert(!annealer.solution().empty());

  return 0;
}
