#include "routes.h"
#include <iostream>
#include <sstream>
#include <fstream>

using std::istream;
using std::ostream;
using std::getline;
using std::string;
using std::stringstream;
using std::vector;
using std::ifstream;
using std::endl;
using std::cerr;
using std::cout;
using std::back_inserter;
using std::copy;

namespace {
  void consume_lines(istream& is, int lines)
  {
    string discard;
    while (lines--)
      getline(is, discard);
  }

  void loadDumas(istream& is, TSPTWWorld& tsp)
  {
    string line;

    consume_lines(is, 5);
    while (is) {
      vector<double> row { 0, 0, 0, 0, 0, 0 };
      int discard;
      stringstream ss;
      getline(is, line);
      ss << line;
      ss >> discard;
      if (999 == discard)
        break;
      ss >> row[0] >> row[1] >> row[2] >> row[3]
         >> row[4] >> row[5];
      tsp.data().push_back(Matrix<double, 1>(row));
    }
  }
}

TSPTWWorld::TSPTWWorld(const char* filename)
{
  ifstream in(filename);
  string line;

  data().reset();

  getline(in, line);
  getline(in, line);
  getline(in, line);
  if (line.substr(0, 2) == "!!") {
    loadDumas(in, *this);
    computeTravelTimes();
  }
}

TSPTWWorld::TSPTWWorld(const std::string& filename)
{
  ifstream in(filename.c_str());
  string line;

  data().reset();

  getline(in, line);
  getline(in, line);
  getline(in, line);
  if (line.substr(0, 2) == "!!") {
    loadDumas(in, *this);
    computeTravelTimes();
  }
}

ostream& operator<<(ostream& os, const TSPTWWorld& w)
{
  for (unsigned int i = 0 ; i < w.data().size() ; i++)
    {
      for (unsigned j = 0 ; j < w.data()[i].size() ; j++)
        os << w.data()[i][j] << "\t";
      os << "\n";
    }
  return os;
}

