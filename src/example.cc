/* Copyright (c) 2004 - 2025, Robert J. Hansen <rjh@sixdemonbag.org>
 * and Tristan D. Thiede (address currently unknown).
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

using std::cout;
using std::endl;

int main() {
  // We start by defining a world:
  auto filename = "/home/rjh/Projects/djinni/src/Dumas-1.set";
  auto world = TravelingSalesmanWorld::loadFromDumasFile(filename);

  // And now we define what valid solutions look like -- in this
  // case, as Traveling Salesman routes through that world:
  auto solution = TravelingSalesmanSolution(world);

  // Next, our annealer's penalty function is given by the
  // Ohlmann-Thomas compression function:
  auto penalty_function = Compression(0.06, 0.0, 0.9999);

  // And we're finally ready to rock and roll.
  Annealer annealer(penalty_function, solution, 0.95, 0.94, 75, 100, 30000);
  annealer.solve();
  cout << annealer << endl;

  return 0;
}
