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

#pragma once
#include <cmath>
#include <concepts>
#include <cstdint>
#include <iosfwd>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace edu::uiowa::tippie::djinni {
//! The contract a PenaltyFunc must satisfy to be used with Annealer.
/*! This makes the interface that was previously only described in prose
   (see Compression and Simulated in penalties.h) into something the
   compiler checks at the point of use, instead of several hundred lines
   deep in a template instantiation error. */
template <class P>
concept PenaltyFunction =
    std::copy_constructible<P> && requires(P p, uint32_t iter) {
      typename P::ReturnType;
      {
        P::defaultReturnTypeValue
      } -> std::convertible_to<typename P::ReturnType>;
      { p(iter) } -> std::convertible_to<typename P::ReturnType>;
    };

//! The contract a SolutionType must satisfy to be used with Annealer.
/*! See TravelingSalesmanSolution in routes.h for the reference
   implementation of this interface. */
template <class S>
concept AnnealingSolution =
    std::copy_constructible<S> && std::assignable_from<S &, const S &> &&
    requires(S &s, const S &cs, S &neighbor, const char *raw,
             std::ostream &os) {
      { cs.getF() } -> std::convertible_to<double>;
      { cs.getP() } -> std::convertible_to<double>;
      s.setP(0.0);
      s.randomize();
      s.compute();
      s.generateNeighbor(neighbor);
      S(raw);
      { os << cs } -> std::same_as<std::ostream &>;
    };

//! A PenaltyFunc which additionally supports the pressure-cap protocol
//! required by Ohlmann-Thomas compression annealing.
/*! This is a structural check against two method names, not a check
   against the concrete type Compression, so any future PenaltyFunc
   exposing the same two methods picks up the same treatment in
   Annealer::initializeParam() automatically, with no specialization
   required.

   For details on Ohlmann-Thomas annealing, please see:

   https://myweb.uiowa.edu/bthoa/DownloadItems/TSPTWpaper4-05-05.pdf */
template <class P>
concept PressureCapped = requires(P &p, double cap) {
  { p.getCapPercentage() } -> std::convertible_to<double>;
  p.setPressureCap(cap);
};

//! A generic Annealer capable of working with a variety of different problem
//! types and penalty generators.
/*! There is no deep magic hidden in this class.  The only behavior that
   varies by PenaltyFunc is the pressure-cap bookkeeping needed by
   Ohlmann-Thomas compression annealing, which initializeParam() branches
   on via the PressureCapped concept rather than by specializing this
   class.

    @author Hansen, Thiede
    @since 2.1
*/
template <PenaltyFunction PenaltyFunc, AnnealingSolution SolutionType>
class Annealer {
public:
  /*! A convenience typedef for accessing the ReturnType of a
  given PenaltyType. */
  typedef PenaltyFunc::ReturnType PenaltyType;

  /*!
  An Annealer constructor which takes all necessary parameters in
  one fell swoop.

  Note that although pfunc is passed in by reference, we create our own local
  copy of it and leave the passed parameter untouched. If you need/want to
  inspect an Annealer's _pfunc member, use .getPenaltyFunc().

  @param pfunc The penalty function to be applied to this annealer
  @param sol A solution, populated randomly, to be applied to this annealer
  @param multT A value in the range 0.0 - 0.9999 representing the temperature
  multiplier
  @param accept A value in the range 0.0 - 0.9999 representing our willingness
  to accept an inferior solution
  @param tBI The terminal best iteration
  @param minIter The minimum number of annealing iterations to apply
  @param maxIter The maximum number of annealing iterations to apply
  */
  Annealer(const PenaltyFunc &pfunc, SolutionType &sol, double multT,
           double accept, uint32_t tBI, uint32_t minIter, uint32_t maxIter)
      : _best(sol), _current(_best), _neighbor(_best), _bestIter(0),
        _iterations(0), _maxIterations(maxIter), _minIterations(minIter),
        _terminalBestIter(tBI), _multiplierT(multT), _acceptProb(accept),
        _currentT(0), _pfunc(pfunc),
        _lambda(PenaltyFunc::defaultReturnTypeValue) {
    validateProbability(multT, "multT");
    validateProbability(accept, "accept");
  }

  /*! An Annealer constructor appropriate for use with PenaltyFuncs which have a
  default constructor.

  @param solution A solution, populated randomly, to be applied to this annealer
  @param multT A value in the range 0.0 - 0.9999 representing the temperature
  multiplier
  @param accept A value in the range 0.0 - 0.9999 representing our willingness
  to accept an inferior solution
  @param tBI The terminal best iteration
  @param minIter The minimum number of annealing iterations to apply
  @param maxIter The maximum number of annealing iterations to apply
  */

  Annealer(SolutionType &solution, double multT, double accept, uint32_t tBI,
           uint32_t minIter, uint32_t maxIter)
      : _best(solution), _current(_best), _neighbor(_best), _bestIter(0),
        _iterations(0), _maxIterations(maxIter), _minIterations(minIter),
        _terminalBestIter(tBI), _multiplierT(multT), _acceptProb(accept),
        _currentT(0), _lambda(PenaltyFunc::defaultReturnTypeValue) {
    validateProbability(multT, "multT");
    validateProbability(accept, "accept");
  }

  /*! An Annealer constructor for use when the parameters will be set after
  initialization.

  Note that although pfunc is passed in by reference, we create our own local
  copy of it and leave the passed parameter untouched. If you need/want to
  inspect an Annealer's _pfunc member, use .getPenaltyFunc().

  @param pfunc The penalty function to be applied to this annealer
  @param sol A solution, populated randomly, to be applied to this annealer
  */
  Annealer(const PenaltyFunc &pfunc, SolutionType &sol)
      : _best(sol), _current(_best), _neighbor(_best), _bestIter(0),
        _iterations(0), _maxIterations(0), _minIterations(0),
        _terminalBestIter(0), _multiplierT(0), _acceptProb(0), _currentT(0),
        _pfunc(pfunc), _lambda(PenaltyFunc::defaultReturnTypeValue) {}

  /*! A no-op destructor.

  This destructor is virtualized to allow end users to subclass off this
  template.  Whether an end user should subclass off this template is a
  different question. */
  virtual ~Annealer() = default;

  /*! Copying is disabled. _best/_current/_neighbor are now plain
  SolutionType values, so a compiler-generated copy would duplicate them
  correctly (TravelingSalesmanSolution's own copy constructor already
  gives each copy an independently-seeded _prng, exactly to avoid
  correlated runs -- see its docs in routes.h). What's still unsafe is
  this class's *own* _prng/_urd below: they have no such protection, so
  a naive copy would copy that engine's state verbatim, and the
  original and the copy would then draw the identical sequence of
  temperature-acceptance decisions from the point of copying onward --
  the same "independent runs secretly correlated" problem the comment
  on _prng/_urd warns about, just one level up. Nothing in this codebase
  copies an Annealer; each constructor already takes its inputs by
  reference and makes its own deep copies internally, so there's no
  legitimate use for a copy to begin with.

  Move is fine (and re-enabled here since declaring the destructor above
  already suppresses the implicitly-declared move operations): moving
  transfers state out of the source rather than duplicating it alongside
  the source, so there's no risk of the two ever running in parallel
  with correlated state. */
  Annealer(const Annealer &) = delete;
  Annealer &operator=(const Annealer &) = delete;
  Annealer(Annealer &&) = default;
  Annealer &operator=(Annealer &&) = default;

  /*! Returns this Annealer's PenaltyFunc.

  This is the Annealer's own local copy, not the object originally passed to
  its constructor -- see the constructor docs for why. Any pressure-cap or
  other tuning the Annealer performs during solve() is visible here, not on
  your original object. */
  PenaltyFunc &getPenaltyFunc() { return _pfunc; }

  /*! Returns the best solution found by the annealer. */
  const SolutionType &best() const { return _best; }

  /*! Returns the current solution in use by the annealer.

  It is unlikely this method will be of use to end users.  Once you hit
  the solve() method, you're on an uninterruptible trip to the end.
  However, in the event you want to subclass and do funky things, you
  have an accessor. */
  const SolutionType &current() const { return _current; }

  /*! For a completely-constructed annealer, initiate the solution process and
   * do not return until termination. */
  void solve() {
    _current = _best;
    _best.setP(1000000);
    // Reset so a second solve() call (e.g. after setSolutionParameters())
    // starts from scratch instead of inheriting state left over from the
    // previous run. This has to happen before tuneTemperature(), which
    // reads _lambda directly; resetting it any later would leave that
    // call tuning against a stale pressure weighting from the prior run.
    _iterations = 0;
    _bestIter = 0;
    _lambda = PenaltyFunc::defaultReturnTypeValue;
    initializeParam();
    tuneTemperature();
    while ((_iterations <= _minIterations) || (_bestIter < _terminalBestIter)) {
      ++_iterations;
      for (uint32_t count = 0; count < _maxIterations; ++count) {
        _current.generateNeighbor(_neighbor);
        testNeigh();
        if ((_current.getP() < _best.getP()) ||
            (_current.getP() == _best.getP() &&
             _current.getF() < _best.getF())) {
          _best = _current;
          _bestIter = 1;
        }
      }
      ++_bestIter;
      updateParam();
    }
  }

  /*! Return a std::string representation of the best solution found by the
  annealer.

  @return A std::string representation of the best solution found by the
  annealer. */
  [[nodiscard]] std::string solution() const {
    std::stringstream ss;
    ss << _best;
    std::string result = ss.str();
    return result;
  }

  /*! Sets the parameters of the annealer.

  @param multT A value in the range 0.0 - 0.9999 representing the temperature
  multiplier
  @param accept A value in the range 0.0 - 0.9999 representing our willingness
  to accept an inferior solution
  @param tBI The terminal best iteration
  @param minIterations The minimum number of annealing iterations to apply
  @param maxIterations The maximum number of annealing iterations to apply */
  void setParameters(double multT, double accept, uint32_t tBI,
                     uint32_t minIterations, uint32_t maxIterations) {
    validateProbability(multT, "multT");
    validateProbability(accept, "accept");
    _multiplierT = multT;
    _acceptProb = accept;
    _terminalBestIter = tBI;
    _minIterations = minIterations;
    _maxIterations = maxIterations;
  }

  /*! Sets the parameters of this annealer's SolutionType by calling that class'
  constructor.

  @param foo A char* containing the solution parameters */
  void setSolutionParameters(const char *foo) {
    _best = SolutionType(foo);
    _current = _best;
    _neighbor = _best;
  }

  /*! Allows for an Annealer object's internal state to be dumped in
  human-readable format to an output stream.

  Please note that this is not meant to be called directly.  Rather, an
  operator<< will be set up as a proxy to invoke this method.

  @param os The output stream to dump it to
  @return The output stream os after the operation completes */
  std::ostream &dump(std::ostream &os) const {
    os << "{\n\t\"best_solution\": {\n\t\t\"base_cost\": " << (_best.getF())
       << ",\n\t\t\"penalty\":   " << (_best.getP()) << "\n\t},\n\t"
       << "\"best_iteration\":          " << _bestIter << ",\n\t"
       << "\"iterations\":              " << _iterations << ",\n\t"
       << "\"count_limit\":             " << _maxIterations << ",\n\t"
       << "\"minimum_iterations\":      " << _minIterations << ",\n\t"
       << "\"sample_size\":             " << _sampleSize << ",\n\t"
       << "\"multiplier\":              " << _multiplierT << ",\n\t"
       << "\"acceptance_probability\":  " << _acceptProb << ",\n\t"
       << "\"terminal_best_iteration\": " << _terminalBestIter << ",\n\t"
       << "\"pressure\":                " << _lambda << "\n}\n";
    return os;
  }

  /*! Returns the cost of the best solution found by the annealer.

  @return The cost of the best solution found by the annealer.*/
  [[nodiscard]] double cost() const { return _best.getF(); }

  /*! Returns the penalty incurred by the best solution found by the annealer.

  @return The penalty incurred by the best solution found by the annealer. */
  [[nodiscard]] double penalty() const { return _best.getP(); }
  /*! Returns the number of the iteration on which the best solution was
  encountered.

  @return The number of the iteration on which the best solution was
  encountered. */
  [[nodiscard]] uint32_t bestIter() const { return _bestIter; }

  /*! Returns the current iteration number.

  @return The current iteration number. */
  [[nodiscard]] uint32_t iterations() const { return _iterations; }

  /*! Returns the maximum number of annealer iterations to run.

  @return The maximum number of annealer iterations to run. */
  [[nodiscard]] uint32_t maxIterations() const { return _maxIterations; }

  /*! Returns the minimum number of annealer iterations to run.

  @return The minimum number of annealer iterations to run. */
  [[nodiscard]] uint32_t minIterations() const { return _minIterations; }
  /*! Returns the temperature multiplier.

  @return The temperature multiplier. */
  [[nodiscard]] double multiplier() const { return _multiplierT; }

  /*! Returns the probability of accepting an inferior move.

  @return The probability of accepting an inferior move. */
  [[nodiscard]] double probability() const { return _acceptProb; }
  /*! Returns the current lambda.

  @return The current lambda. */
  [[nodiscard]] PenaltyType getLambda() const { return _lambda; }

protected:
  /*! Rejects a multT/accept value outside the documented (0.0, 1.0) range.

  Both feed into initializeParam()'s `-sum / log(_acceptProb)`: 0.0 or
  1.0 make log() return -inf/0, and anything outside [0.0, 1.0] isn't a
  meaningful probability/multiplier to begin with. Left unchecked, an
  out-of-range value doesn't fail loudly -- it silently produces
  +-inf/NaN that propagates through tuneTemperature()'s exit condition
  and quietly skips real temperature tuning instead of erroring.

  @param v The value to check
  @param name The parameter name, for the exception message
  @throws std::invalid_argument if v is not in (0.0, 1.0) */
  static void validateProbability(double v, const char *name) {
    if (!(v > 0.0 && v < 1.0))
      throw std::invalid_argument(std::string("Annealer: ") + name +
                                  " must be in the range (0.0, 1.0)");
  }

  /*! Performs housekeeping to make sure our parameters are properly set before
   * entering annealing runs.

   The pressure-cap bookkeeping required by Ohlmann-Thomas compression
   annealing is only compiled in for PenaltyFuncs that satisfy the
   PressureCapped concept; for every other PenaltyFunc the `if
   constexpr` branches below are discarded entirely. */
  void initializeParam() {
    double lambda1 = 0, sum = 0;
    [[maybe_unused]] double scale = 0, cap = 0;
    if constexpr (PressureCapped<PenaltyFunc>)
      scale = _pfunc.getCapPercentage() / (1 - _pfunc.getCapPercentage());

    for (uint32_t j = 0; j < _sampleSize - 1; j += 2) {
      _current.randomize();
      _current.compute();
      _current.generateNeighbor(_neighbor);
      if constexpr (PressureCapped<PenaltyFunc>) {
        double lambda0;
        if (_current.getP() > 0) {
          lambda0 = (_current.getF() / _current.getP()) * scale;
          if (lambda0 > cap)
            cap = lambda0;
        }
        if (_neighbor.getP() > 0) {
          lambda0 = (_neighbor.getF() / _neighbor.getP()) * scale;
          if (lambda0 > cap)
            cap = lambda0;
        }
      }
      double u = (_current.getF() + lambda1 * _current.getP()) -
                 (_neighbor.getF() + lambda1 * _neighbor.getP());
      sum += u > 0 ? u : (-1 * u);
    }
    if constexpr (PressureCapped<PenaltyFunc>)
      _pfunc.setPressureCap(cap);

    sum /= _sampleSize;
    _currentT = ((-1 * sum) / log(_acceptProb));
  }

  /*! Runs some initial annealing iterations in order to set the temperature to
   * the proper initial value. */
  void tuneTemperature() {
    int acceptedWorse, uphill;
    double ratio;
    do {
      acceptedWorse = uphill = 0;
      for (uint32_t count = 0; count < _maxIterations; count++) {
        _current.generateNeighbor(_neighbor);
        double delta = (_neighbor.getF() + _lambda * _neighbor.getP()) -
                       (_current.getF() + _lambda * _current.getP());
        if (delta < 0)
          std::swap(_current, _neighbor);
        else {
          uphill++;
          double u = 0 - delta / _currentT;
          if (randomReal() < exp(u)) {
            std::swap(_current, _neighbor);
            acceptedWorse++;
          }
        }
        if ((_current.getP() < _best.getP()) ||
            ((_current.getP() == _best.getP()) &&
             (_current.getF() < _best.getF())))
          _best = _current;
      }
      // If every move this pass was improving, there were no uphill moves
      // to measure acceptance of, and acceptedWorse/uphill would be the
      // undefined 0/0. Treat "nothing but improving moves" as already
      // meeting the acceptance target, rather than dividing by zero.
      ratio = uphill == 0 ? _acceptProb
                          : static_cast<double>(acceptedWorse) /
                                static_cast<double>(uphill);
      if (ratio < _acceptProb)
        _currentT = 1.5 * _currentT;
    } while (ratio < _acceptProb);
  }

  /*! Tests a neighbor for superiority or inferiority, and may update our
   * _current solution based on the result. */
  void testNeigh() {
    double delta = (_neighbor.getF() + _lambda * _neighbor.getP()) -
                   (_current.getF() + _lambda * _current.getP());
    if (delta < 0)
      std::swap(_current, _neighbor);
    else {
      double u = 0 - delta / _currentT;
      if (randomReal() < exp(u))
        std::swap(_current, _neighbor);
    }
  }

  /*! Updates the temperature and lambda each iteration. */
  void updateParam() {
    _currentT = _multiplierT * _currentT;
    _lambda = _pfunc(_iterations);
  }

  SolutionType _best, _current, _neighbor;

  uint32_t _bestIter, _iterations, _maxIterations{}, _minIterations{},
      _terminalBestIter;
  static constexpr int _sampleSize = 10000;
  double _multiplierT{}, _acceptProb{}, _currentT{};
  PenaltyFunc _pfunc;
  PenaltyType _lambda;

  // Per-instance, not static: a shared engine across every Annealer of a
  // given type would race if two Annealers ran concurrently on separate
  // threads, and would make every Annealer's "independent" run secretly
  // correlated with every other's.
  std::mt19937_64 _prng{std::random_device{}()};
  std::uniform_real_distribution<> _urd{0.0, 1.0};
  double randomReal() { return _urd(_prng); }
};

/*! An overridden operator<< which serves as a proxy for an Annealer's dump()
   method.

    @param os The output stream to write the Annealer to
    @param engine The Annealer to be written
    @return The output stream after the Annealer is written */
template <class PenaltyFunc, class SolutionType>
std::ostream &operator<<(std::ostream &os,
                         const Annealer<PenaltyFunc, SolutionType> &engine) {
  return engine.dump(os);
}
} // namespace edu::uiowa::tippie::djinni
