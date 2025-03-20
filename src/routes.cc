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

#include "djinni/routes.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>

using std::back_inserter;
using std::cerr;
using std::copy;
using std::cout;
using std::endl;
using std::getline;
using std::ifstream;
using std::istream;
using std::istreambuf_iterator;
using std::ostream;
using std::regex;
using std::smatch;
using std::stod;
using std::stoi;
using std::string;
using std::stringstream;
using std::vector;

namespace {
regex drx("^\\s*(\\d+)"
          "(\\s+[+-]?[0-9]*[.]?[0-9]+)"
          "(\\s+[+-]?[0-9]*[.]?[0-9]+)"
          "(\\s+[+-]?[0-9]*[.]?[0-9]+)"
          "(\\s+[+-]?[0-9]*[.]?[0-9]+)"
          "(\\s+[+-]?[0-9]*[.]?[0-9]+)"
          "(\\s+[+-]?[0-9]*[.]?[0-9]+)"
          "\\s*$");
} // namespace

TravelingSalesmanWorld::TravelingSalesmanWorld() {}

TravelingSalesmanWorld
TravelingSalesmanWorld::loadFromDumasFile(string filename) {
  ifstream in(filename);
  string str(istreambuf_iterator<char>{in}, istreambuf_iterator<char>{});
  return loadFromDumasString(str);
}

TravelingSalesmanWorld
TravelingSalesmanWorld::loadFromDumasString(string dumasStr) {
  TravelingSalesmanWorld tsp;
  smatch match;
  size_t pos = 0;
  while ((pos = dumasStr.find("\n")) != string::npos) {
    string line = dumasStr.substr(0, pos);
    if (std::regex_match(line, match, drx)) {
      if (999 == stoi(match[1].str()))
        break;
      vector<double> row(6);
      for (uint32_t i = 0; i < 6; i += 1)
        row[i] = stod(match[i + 2].str());
      tsp.data().push_back(Matrix<double, 1>(row));
    }
    dumasStr.erase(0, pos + 1);
  }
  tsp.computeTravelTimes();
  return tsp;
}
