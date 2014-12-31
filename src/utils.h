/* In the beginning, this was a complete rewrite of
   NISHIMURA Takuji and MATSUMOTO Makoto's C library for the
   Mersenne Twister.  When we introduced a dependency on 
   Boost in version 2.x, this file was put on the chopping
   block.  Boost provides its own Mersenne Twister; why 
   shouldn't we just use theirs?  Hence, this class is now a
   thin compatibility wrapper around Boost's Twister.

   (c) 2005, the University of Iowa; (c) 2009, Robert J. Hansen.
   This code is redistributable under the terms found in the
   LICENSE file. */

#ifndef TWISTER_H
#define TWISTER_H
#include <sys/types.h>
#include <boost/random.hpp>

//! The Mersenne Twister.
/** This is a fairly good pseudorandom number generator for Monte
    Carlo simulations, but lousy for other purposes (including
    cryptographic ones).*/

class Twister {
 public:
  static void reseed(u_int32_t seed = 4397) { Twister::mt.seed(seed); }
  //! Generates a real number in the range [0.0, 1).
  static double generateDouble() { return Twister::rng(); }

  static boost::mt19937 mt;
  static boost::uniform_real<> real;
  static boost::variate_generator<boost::mt19937&, boost::uniform_real<> > rng;
  static boost::random_number_generator<boost::variate_generator<boost::mt19937&, boost::uniform_real<> > > stl_rng;
};

#endif

