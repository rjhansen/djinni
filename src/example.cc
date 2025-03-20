/* Copyright (c) 2004 - 2025, Robert J. Hansen <rjh@sixdemonbag.org>
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

#include "djinni.h"
#include <filesystem>

using std::cerr;
using std::cout;
using std::endl;
using std::filesystem::exists;

int main() {
  const char *filename{"Dumas-1.set"};
  if (!exists(filename)) {
    cerr << "Error: couldn't find the file '" << filename << "'." << endl;
    return 1;
  }
  // We start by defining a world:
  auto world = TravelingSalesmanWorld::loadFromDumasFile(filename);

  // And now we define our initial (bad) guess at a solution to this
  // world:
  auto initial_solution = TravelingSalesmanSolution(world);

  // Next, our annealer's penalty function is given by the
  // Ohlmann-Thomas compression function:
  auto penalty_function = Compression(0.06, 0.0, 0.9999);

  // And we're finally ready to rock and roll.
  auto annealer =
      Annealer(penalty_function, initial_solution, 0.95, 0.94, 75, 100, 30000);
  annealer.solve();
  cout << annealer << endl;

  return 0;
}
