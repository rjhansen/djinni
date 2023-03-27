/* (c) 2005, the University of Iowa; (c) 2023, Robert J. Hansen.
   This code is redistributable under the terms found in the
   LICENSE file. */

#ifndef UTILS_H
#define UTILS_H
#include <random>

extern std::random_device rd;
extern std::mt19937 prng;
double randomReal();

#endif
