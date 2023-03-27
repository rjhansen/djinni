#ifndef ANNEALERS_H
#define ANNEALERS_H

#include "penalties.h"
#include "utils.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iosfwd>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdint.h>
#include <string>

//! A generic Annealer capable of working with a variety of different problem
//! types and penalty generators.
/*! There is no deep magic hidden in this class.  You may wish to check the
   specialization for PenaltyFunc = Compressed, which follows this class
   declaration.

    @author Hansen, Thiede
    @since 2.1
*/
template <class PenaltyFunc, class SolutionType> class Annealer {
public:
  /*! A convenience typedef for accessing the ReturnType of a
  given PenaltyType. */
  typedef typename PenaltyFunc::ReturnType PenaltyType;

  /*!
  An Annealer constructor which takes all necessary parameters in
  one fell swoop.

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
  Annealer(PenaltyFunc &pfunc, SolutionType &sol, double multT, double accept,
           uint32_t tBI, uint32_t minIter, uint32_t maxIter)
      : _best(new SolutionType(sol)), _current(new SolutionType(*_best)),
        _neighbor(new SolutionType(*_best)), _bestIter(0), _iterations(0),
        _terminalBestIter(0), _pfunc(pfunc),
        _lambda(PenaltyFunc::defaultReturnTypeValue) {
    setParameters(multT, accept, tBI, minIter, maxIter);
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
      : _best(new SolutionType(solution)), _current(new SolutionType(*_best)),
        _neighbor(new SolutionType(*_best)), _bestIter(0), _iterations(0),
        _terminalBestIter(0) {
    setParameters(multT, accept, tBI, minIter, maxIter);
  }

  /*! An Annealer constructor for use when the parameters will be set after
  initialization.

  @param pfunc The penalty function to be applied to this annealer
  @param sol A solution, populated randomly, to be applied to this annealer
  */
  Annealer(PenaltyFunc &pfunc, SolutionType &sol)
      : _best(new SolutionType(sol)), _current(new SolutionType(*_best)),
        _neighbor(new SolutionType(*_best)), _bestIter(0), _iterations(0),
        _terminalBestIter(0), _pfunc(pfunc) {}

  /*! A no-op destructor.

  This destructor is virtualized to allow end users to subclass off this
  template.  Whether an end user should subclass off this template is a
  different question. */
  virtual ~Annealer() {}

  /*! Returns this Annealer's PenaltyFunc */
  PenaltyFunc &getPenaltyFunc() { return _pfunc; }

  /*! Returns the best solution found by the annealer. */
  const SolutionType &best() const { return *_best; }

  /*! Returns the current solution in use by the annealer.

  It is unlikely this method will be of use to end users.  Once you hit
  the solve() method, you're on an uninterruptible trip to the end.
  However, in the event you want to subclass and do funky things, you
  have an accessor. */
  const SolutionType &current() const { return *_current; }

  /*! For a completely-constructed annealer, initiate the solution process and
   * do not return until termination. */
  void solve() {
    *_current = *_best;
    _best->setP(1000000);
    initializeParam();
    tuneTemperature();
    _iterations = 0;
    while ((_iterations <= _minIterations) || (_bestIter < _terminalBestIter)) {
      ++_iterations;
      for (uint32_t count = 0; count < _maxIterations; ++count) {
        _current->generateNeighbor(*_neighbor);
        testNeigh();
        if (_current->getP() < _best->getP()) {
          (*_best) = (*_current);
          _bestIter = 1;
        } else if (_current->getP() == _best->getP() &&
                   _current->getF() < _best->getF()) {
          (*_best) = (*_current);
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
  std::string solution() const {
    std::stringstream ss;
    ss << (*_best);
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
    _best = std::shared_ptr<SolutionType>(new SolutionType(foo));
    _current = std::shared_ptr<SolutionType>(new SolutionType(*_best));
    _neighbor = std::shared_ptr<SolutionType>(new SolutionType(*_best));
  }

  /*! Allows for an Annealer object's internal state to be dumped in
  human-readable format to an output stream.

  Please note that this is not meant to be called directly.  Rather, an
  operator<< will be set up as a proxy to invoke this method.

  @param os The output stream to dump it to
  @return The output stream os after the operation completes */
  std::ostream &dump(std::ostream &os) const {
    os << "Best solution:      " << (_best->getF()) << "\t" << (_best->getP())
       << std::endl;
    os << "Best iter:          " << _bestIter << std::endl;
    os << "Iterations:         " << _iterations << std::endl;
    os << "Count limit:        " << _maxIterations << std::endl;
    os << "Minimum iterations: " << _minIterations << std::endl;
    os << "Sample size:        " << _sampleSize << std::endl;
    os << "Multiplier:         " << _multiplierT << std::endl;
    os << "Accept probability: " << _acceptProb << std::endl;
    os << "Terminal best iter: " << _terminalBestIter << std::endl;
    os << "Pressure:           " << _lambda << std::endl;
    return os;
  }

  /*! Returns the cost of the best solution found by the annealer.

  @return The cost of the best solution found by the annealer.*/
  double cost() const { return _best->getF(); }

  /*! Returns the penalty incurred by the best solution found by the annealer.

  @return The penalty incurred by the best solution found by the annealer. */
  double penalty() const { return _best->getP(); }
  /*! Returns the number of the iteration on which the best solution was
  encountered.

  @return The number of the iteration on which the best solution was
  encountered. */
  uint32_t bestIter() const { return _bestIter; }

  /*! Returns the current iteration number.

  @return The current iteration number. */
  uint32_t iterations() const { return _iterations; }

  /*! Returns the maximum number of annealer iterations to run.

  @return The maximum number of annealer iterations to run. */
  uint32_t maxIterations() const { return _maxIterations; }

  /*! Returns the minimum number of annealer iterations to run.

  @return The minimum number of annealer iterations to run. */
  uint32_t minIterations() const { return _minIterations; }
  /*! Returns the temperature multiplier.

  @return The temperature multiplier. */
  double multiplier() const { return _multiplierT; }

  /*! Returns the probability of accepting an inferior move.

  @return The probability of accepting an inferior move. */
  double probability() const { return _acceptProb; }
  /*! Returns the current lambda.

  @return The current lambda. */
  PenaltyType getLambda() const { return _lambda; }

protected:
  /*! Performs housekeeping to make sure our parameters are properly set before
   * entering annealing runs. */
  void initializeParam() {
    double lambda1 = 0, sum = 0;
    for (uint32_t j = 0; j < _sampleSize - 1; j += 2) {
      _current->randomize();
      _current->compute();
      _current->generateNeighbor(*_neighbor);
      double u = (_current->getF() + lambda1 * _current->getP()) -
                 (_neighbor->getF() + lambda1 * _neighbor->getP());
      sum += u > 0 ? u : (-1 * u);
    }

    sum /= _sampleSize;
    _currentT = ((-1 * sum) / log(_acceptProb));
  }

  /*! Runs some initial annealing iterations in order to set the temperature to
   * the proper initial value. */
  void tuneTemperature() {
    int acceptedWorse, uphill;
    do {
      acceptedWorse = uphill = 0;
      for (uint32_t count = 0; count < _maxIterations; count++) {
        _current->generateNeighbor(*_neighbor);
        double delta, u;
        delta = (_neighbor->getF() + _lambda * _neighbor->getP()) -
                (_current->getF() + _lambda * _current->getP());
        if (delta < 0)
          _current.swap(_neighbor);
        else {
          uphill++;
          u = 0 - delta / _currentT;
          if (randomReal() < exp(u)) {
            _current.swap(_neighbor);
            acceptedWorse++;
          }
        }
        if (_current->getP() < _best->getP())
          *_best = *_current;
        else if ((_current->getP() == _best->getP()) &&
                 (_current->getF() < _best->getF()))
          *_best = *_current;
      }
      if ((static_cast<double>(acceptedWorse) / static_cast<double>(uphill)) <
          _acceptProb)
        _currentT = 1.5 * _currentT;
    } while ((static_cast<double>(acceptedWorse) /
              static_cast<double>(uphill)) < _acceptProb);
  }

  /*! Tests a neighbor for superiority or inferiority, and may update our
   * _current solution based on the result. */
  void testNeigh() {
    double delta, u;
    delta = (_neighbor->getF() + _lambda * _neighbor->getP()) -
            (_current->getF() + _lambda * _current->getP());
    if (delta < 0)
      _current.swap(_neighbor);
    else {
      u = 0 - delta / _currentT;
      if (randomReal() < exp(u))
        _current.swap(_neighbor);
    }
  }

  /*! Updates the temperature and lambda each iteration. */
  void updateParam() {
    _currentT = _multiplierT * _currentT;
    _lambda = _pfunc(_iterations);
  }

  std::shared_ptr<SolutionType> _best, _current, _neighbor;

  uint32_t _bestIter, _iterations, _maxIterations, _minIterations,
      _terminalBestIter;
  static const int _sampleSize = 10000;
  double _multiplierT, _acceptProb, _currentT;
  PenaltyFunc _pfunc;
  PenaltyType _lambda;
};

//! An Annealer specialized for Ohlmann-Thomas compressed annealing.
/*! For details on Ohlmann-Thomas annealing, please see [FIXME REFERENCE].

    @author Hansen, Thiede
    @since 2.1
*/
template <class SolutionType> class Annealer<Compressed, SolutionType> {
public:
  typedef Compressed PenaltyFunc;
  typedef typename PenaltyFunc::ReturnType PenaltyType;

  /*!
  An Annealer constructor which takes all necessary parameters in
  one fell swoop.

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
  Annealer(PenaltyFunc &pfunc, SolutionType &sol, double multT, double accept,
           uint32_t tBI, uint32_t minIter, uint32_t maxIter)
      : _best(new SolutionType(sol)), _current(new SolutionType(*_best)),
        _neighbor(new SolutionType(*_best)), _bestIter(0), _iterations(0),
        _terminalBestIter(0), _pfunc(pfunc),
        _lambda(PenaltyFunc::defaultReturnTypeValue) {
    setParameters(multT, accept, tBI, minIter, maxIter);
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
      : _best(new SolutionType(solution)), _current(new SolutionType(*_best)),
        _neighbor(new SolutionType(*_best)), _bestIter(0), _iterations(0),
        _terminalBestIter(0) {
    setParameters(multT, accept, tBI, minIter, maxIter);
  }

  /*! An Annealer constructor for use when the parameters will be set after
  initialization.

  @param pfunc The penalty function to be applied to this annealer
  @param sol A solution, populated randomly, to be applied to this annealer
  */
  Annealer(PenaltyFunc &pfunc, SolutionType &sol)
      : _best(new SolutionType(sol)), _current(new SolutionType(*_best)),
        _neighbor(new SolutionType(*_best)), _bestIter(0), _iterations(0),
        _terminalBestIter(0), _pfunc(pfunc) {}

  /*! A no-op destructor.

  This destructor is virtualized to allow end users to subclass off this
  template.  Whether an end user should subclass off this template is a
  different question. */
  virtual ~Annealer() {}

  /*! Returns this Annealer's PenaltyFunc */
  PenaltyFunc &getPenaltyFunc() { return _pfunc; }

  /*! Returns the best solution found by the annealer. */
  const SolutionType &best() const { return *_best; }

  /*! Returns the current solution in use by the annealer.

  It is unlikely this method will be of use to end users.  Once you hit
  the solve() method, you're on an uninterruptible trip to the end.
  However, in the event you want to subclass and do funky things, you
  have an accessor. */
  const SolutionType &current() const { return *_current; }

  /*! For a completely-constructed annealer, initiate the solution process and
   * do not return until termination. */
  void solve() {
    *_current = *_best;
    _best->setP(1000000);
    initializeParam();
    tuneTemperature();
    _iterations = 0;
    while ((_iterations <= _minIterations) || (_bestIter < _terminalBestIter)) {
      ++_iterations;
      for (uint32_t count = 0; count < _maxIterations; ++count) {
        _current->generateNeighbor(*_neighbor);
        testNeigh();
        if (_current->getP() < _best->getP()) {
          (*_best) = (*_current);
          _bestIter = 1;
        } else if (_current->getP() == _best->getP() &&
                   _current->getF() < _best->getF()) {
          (*_best) = (*_current);
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
  std::string solution() const {
    std::stringstream ss;
    ss << (*_best);
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
    _best = std::shared_ptr<SolutionType>(new SolutionType(foo));
    _current = std::shared_ptr<SolutionType>(new SolutionType(*_best));
    _neighbor = std::shared_ptr<SolutionType>(new SolutionType(*_best));
  }

  /*! Allows for an Annealer object's internal state to be dumped in
  human-readable format to an output stream.

  Please note that this is not meant to be called directly.  Rather, an
  operator<< will be set up as a proxy to invoke this method.

  @param os The output stream to dump it to
  @return The output stream os after the operation completes */
  std::ostream &dump(std::ostream &os) const {
    os << "Best solution:      " << (_best->getF()) << "\t" << (_best->getP())
       << std::endl;
    os << "Best iter:          " << _bestIter << std::endl;
    os << "Iterations:         " << _iterations << std::endl;
    os << "Count limit:        " << _maxIterations << std::endl;
    os << "Minimum iterations: " << _minIterations << std::endl;
    os << "Sample size:        " << _sampleSize << std::endl;
    os << "Multiplier:         " << _multiplierT << std::endl;
    os << "Accept probability: " << _acceptProb << std::endl;
    os << "Terminal best iter: " << _terminalBestIter << std::endl;
    os << "Pressure:           " << _lambda << std::endl;
    return os;
  }

  /*! Returns the cost of the best solution found by the annealer.

  @return The cost of the best solution found by the annealer.*/
  double cost() const { return _best->getF(); }

  /*! Returns the penalty incurred by the best solution found by the annealer.

  @return The penalty incurred by the best solution found by the annealer. */
  double penalty() const { return _best->getP(); }
  /*! Returns the number of the iteration on which the best solution was
  encountered.

  @return The number of the iteration on which the best solution was
  encountered. */
  uint32_t bestIter() const { return _bestIter; }

  /*! Returns the current iteration number.

  @return The current iteration number. */
  uint32_t iterations() const { return _iterations; }

  /*! Returns the maximum number of annealer iterations to run.

  @return The maximum number of annealer iterations to run. */
  uint32_t maxIterations() const { return _maxIterations; }

  /*! Returns the minimum number of annealer iterations to run.

  @return The minimum number of annealer iterations to run. */
  uint32_t minIterations() const { return _minIterations; }
  /*! Returns the temperature multiplier.

  @return The temperature multiplier. */
  double multiplier() const { return _multiplierT; }

  /*! Returns the probability of accepting an inferior move.

  @return The probability of accepting an inferior move. */
  double probability() const { return _acceptProb; }
  /*! Returns the current lambda.

  @return The current lambda. */
  PenaltyType getLambda() const { return _lambda; }

protected:
  /*! Performs housekeeping to make sure our parameters are properly set before
   * entering annealing runs. */
  void initializeParam() {
    double lambda1 = 0, sum = 0;
    double lambda0 = 0, cap = 0;
    double solutionScalePressure =
        _pfunc.getCapPercentage() / (1 - _pfunc.getCapPercentage());
    for (uint32_t j = 0; j < _sampleSize - 1; j += 2) {
      _current->randomize();
      _current->compute();
      _current->generateNeighbor(*_neighbor);
      if (_current->getP() > 0)
        lambda0 = (_current->getF() / _current->getP()) * solutionScalePressure;
      if (lambda0 > cap)
        cap = lambda0;
      if (_neighbor->getP() > 0)
        lambda0 =
            (_neighbor->getF() / _neighbor->getP()) * solutionScalePressure;
      if (lambda0 > cap)
        cap = lambda0;
      double u = (_current->getF() + lambda1 * _current->getP()) -
                 (_neighbor->getF() + lambda1 * _neighbor->getP());
      sum += u > 0 ? u : (-1 * u);
    }

    _pfunc.setPressureCap(cap);
    sum /= _sampleSize;
    _currentT = ((-1 * sum) / log(_acceptProb));
  }

  /*! Run some initial annealing iterations in order to set the temperature to
   * the proper initial value. */
  void tuneTemperature() {
    int acceptedWorse, uphill;
    uint32_t iterations = 0;
    do {
      ++iterations;
      acceptedWorse = uphill = 0;
      for (uint32_t count = 0; count < _maxIterations; count++) {
        _current->generateNeighbor(*_neighbor);
        double delta, u;
        delta = (_neighbor->getF() + _lambda * _neighbor->getP()) -
                (_current->getF() + _lambda * _current->getP());
        if (delta < 0)
          _current.swap(_neighbor);
        else {
          uphill++;
          u = 0 - delta / _currentT;
          double generated = randomReal();
          if (generated < exp(u)) {
            _current.swap(_neighbor);
            acceptedWorse++;
          }
        }
        if (_current->getP() < _best->getP())
          *_best = *_current;
        else if ((_current->getP() == _best->getP()) &&
                 (_current->getF() < _best->getF()))
          *_best = *_current;
      }
      if ((static_cast<double>(acceptedWorse) / static_cast<double>(uphill)) <
          _acceptProb)
        _currentT = 1.5 * _currentT;
    } while ((static_cast<double>(acceptedWorse) /
              static_cast<double>(uphill)) < _acceptProb);
  }

  /*! Tests a neighbor for superiority or inferiority, and may update our
   * _current solution based on the result. */
  void testNeigh() {
    double delta, u;
    delta = (_neighbor->getF() + _lambda * _neighbor->getP()) -
            (_current->getF() + _lambda * _current->getP());
    if (delta < 0)
      _current.swap(_neighbor);
    else {
      u = 0 - delta / _currentT;
      if (randomReal() < exp(u))
        _current.swap(_neighbor);
    }
  }

  /*! Updates the temperature and lambda each iteration. */
  void updateParam() {
    _currentT = _multiplierT * _currentT;
    _lambda = _pfunc(_iterations);
  }

  std::shared_ptr<SolutionType> _best, _current, _neighbor;

  uint32_t _bestIter, _iterations, _maxIterations, _minIterations,
      _terminalBestIter;
  static const int _sampleSize = 10000;
  double _multiplierT, _acceptProb, _currentT;
  PenaltyFunc _pfunc;
  PenaltyType _lambda;
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

#endif
