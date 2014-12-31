#include "utils.h"
#include <ctime>

using boost::mt19937;
using boost::uniform_real;
using boost::variate_generator;
using boost::random_number_generator;
using std::time;

typedef variate_generator<mt19937&, uniform_real<> > generator;

mt19937 Twister::mt = mt19937(static_cast<u_int32_t>(time(0)));
uniform_real<> Twister::real = uniform_real<>(0, 1);
generator Twister::rng = generator(Twister::mt, Twister::real);
random_number_generator<generator> Twister::stl_rng = random_number_generator<generator>(Twister::rng);

