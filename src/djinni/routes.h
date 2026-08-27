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

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <memory>
#include <random>
#include <regex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace edu::uiowa::tippie::djinni {
//! A recursive templatized structure representing a matrix of arbitrary
//! dimensionality.

/*! Originally we had our own Matrix class to provide arbitrary dimensions,
    but implemented it via runtime checks and nonrecursive structures.  As
    it turns out this was precisely the wrong thing to do.  Switching to a
    recursive template resulted in immense performance improvements.

    @author Hansen
    @since 2.2.2
*/
template <typename T, const int N> class Matrix {
public:
  typedef T value_type;

  Matrix() = default;

  explicit Matrix(const std::vector<Matrix<T, N - 1>> &matrix)
      : _matrix(matrix) {}
#ifdef USE_BOUNDS_CHECKING
  const Matrix<T, N - 1> &operator[](const uint32_t n) const {
    return _matrix.at(n);
  }
  Matrix<T, N - 1> &operator[](const uint32_t n) { return _matrix.at(n); }
#else
  const Matrix<T, N - 1> &operator[](const uint32_t n) const {
    return _matrix[n];
  }

  Matrix<T, N - 1> &operator[](const uint32_t n) { return _matrix[n]; }
#endif
  static uint32_t dimensions() { return Matrix<T, N>::DIMENSIONS; }

  void reset() {
    for (uint32_t i = 0; i < _matrix.size(); i += 1)
      _matrix[i].reset();
  }

  [[nodiscard]] uint32_t size() const { return _matrix.size(); }
  void push_back(const Matrix<T, N - 1> &matrix) { _matrix.push_back(matrix); }
  void resize(const uint32_t n) { _matrix.resize(n, Matrix<T, N - 1>()); }

protected:
  static const uint32_t DIMENSIONS = N;
  std::vector<Matrix<T, N - 1>> _matrix;
};

//! A recursive templatized structure representing a one-dimensional matrix.

/*! Originally we had our own Matrix class to provide arbitrary dimensions,
    but implemented it via runtime checks and nonrecursive structures.  As
    it turns out this was precisely the wrong thing to do.  Switching to a
    recursive template resulted in immense performance improvements.

    @author Hansen
    @since 2.2.2
*/
template <typename T> class Matrix<T, 1> {
public:
  typedef T value_type;

  Matrix() : _matrix() {}

  explicit Matrix(const std::vector<T> &vec) : _matrix(vec) {}
#ifdef USE_BOUNDS_CHECKING
  const T &operator[](const uint32_t n) const { return _matrix.at(n); }
  T &operator[](const uint32_t n) { return _matrix.at(n); }
#else
  const T &operator[](const uint32_t n) const { return _matrix[n]; }
  T &operator[](const uint32_t n) { return _matrix[n]; }
#endif
  static uint32_t dimensions() { return Matrix<T, 1>::DIMENSIONS; }
  void reset() { _matrix.clear(); }
  [[nodiscard]] uint32_t size() const { return _matrix.size(); }
  void push_back(const std::vector<T> &vec) { _matrix.push_back(vec); }
  void resize(const uint32_t n) { _matrix.resize(n, T()); }

protected:
  std::vector<T> _matrix;
  static constexpr uint32_t DIMENSIONS = 1;
};

//! A class representing an instance of the Traveling Salesman Problem with Time
//! Windows.

/*! @author Hansen, Ohlmann, Thomas
    @since 1.0
*/
class TravelingSalesmanWorld {
public:
  TravelingSalesmanWorld() = default;
  virtual ~TravelingSalesmanWorld() = default;

  /*! Copy operations stay available (TravelingSalesmanSolution's
  WorldType-reference constructor needs to copy-construct its own World
  from the one it's given), but declaring the move operations below would
  otherwise implicitly delete them, so they're spelled out here too.

  Move operations are new: without them, moving a TravelingSalesmanWorld
  silently fell back to copying it instead (a user-declared destructor
  suppresses the implicitly-declared move constructor/assignment, same
  as it did for Annealer and TravelingSalesmanSolution before those were
  fixed) -- meaning every *_w = WorldType::loadFromDumasFile(...) in
  TravelingSalesmanSolution's char* constructor deep-copied the full
  O(n^2) _matrix/_timeMatrix from the temporary and then threw the
  temporary away, rather than just taking its buffers. No member here
  needs special handling the way TravelingSalesmanSolution's _prng/_dis
  did, so plain defaults are correct. */
  TravelingSalesmanWorld(const TravelingSalesmanWorld &) = default;
  TravelingSalesmanWorld &operator=(const TravelingSalesmanWorld &) = default;
  TravelingSalesmanWorld(TravelingSalesmanWorld &&) = default;
  TravelingSalesmanWorld &operator=(TravelingSalesmanWorld &&) = default;

  static TravelingSalesmanWorld loadFromDumasFile(std::string filename) {
    std::ifstream in(filename);
    if (!in.is_open())
      throw std::runtime_error("TravelingSalesmanWorld: couldn't open file '" +
                               filename + "'");
    std::string str(std::istreambuf_iterator<char>{in},
                    std::istreambuf_iterator<char>{});
    return loadFromDumasString(str);
  }

  static TravelingSalesmanWorld
  loadFromDumasString(const std::string &dumasStr) {
    TravelingSalesmanWorld tsp;
    std::smatch match;
    size_t start = 0;
    static std::regex drx("^\\s*(\\d+)"
                          "(\\s+[+-]?[0-9]*[.]?[0-9]+)"
                          "(\\s+[+-]?[0-9]*[.]?[0-9]+)"
                          "(\\s+[+-]?[0-9]*[.]?[0-9]+)"
                          "(\\s+[+-]?[0-9]*[.]?[0-9]+)"
                          "(\\s+[+-]?[0-9]*[.]?[0-9]+)"
                          "(\\s+[+-]?[0-9]*[.]?[0-9]+)"
                          "\\s*$");
    size_t pos;
    while ((pos = dumasStr.find('\n', start)) != std::string::npos) {
      std::string line = dumasStr.substr(start, pos - start);
      start = pos + 1;
      if (std::regex_match(line, match, drx)) {
        // The regex only confirms the fields look numeric, not that they
        // fit in an int/double -- stoi/stod throw std::out_of_range on a
        // field with too many digits. Turn that into a well-defined,
        // catchable error instead of letting it propagate as an unrelated
        // library exception (or, uncaught, terminate the process).
        try {
          if (999 == std::stoi(match[1].str()))
            break;
          std::vector<double> row(6);
          for (uint32_t i = 0; i < 6; i += 1)
            row[i] = std::stod(match[i + 2].str());
          tsp.data().push_back(Matrix<double, 1>(row));
        } catch (const std::out_of_range &) {
          throw std::invalid_argument(
              "TravelingSalesmanWorld: numeric field out of range in line: " +
              line);
        }
      }
    }
    tsp.computeTravelTimes();
    return tsp;
  }

  [[nodiscard]] const Matrix<double, 2> &travelTimes() const {
    return _timeMatrix;
  }
  [[nodiscard]] const std::vector<double> &lowDeadlines() const {
    return _lowdeadlines;
  }
  [[nodiscard]] const std::vector<double> &deadlines() const {
    return _deadlines;
  }

  //! Returns a const-reference to the Matrix used to store this world's data.
  [[nodiscard]] const Matrix<double, 2> &data() const { return _matrix; }

  //! Returns a reference to the Matrix used to store this world's data.
  [[nodiscard]] Matrix<double, 2> &data() { return _matrix; }

protected:
  Matrix<double, 2> _timeMatrix;
  Matrix<double, 2> _matrix;
  std::vector<double> _lowdeadlines, _deadlines;

  virtual void computeTravelTimes() {
    uint32_t numCustomers = _matrix.size();
    _timeMatrix.resize(numCustomers);
#ifdef DJINNI_FASTER
    // Travel times are plain Euclidean distances, which already satisfy the
    // triangle inequality exactly: no route via a third customer k is ever
    // shorter than the direct one, so no all-pairs relaxation is needed.
    // (Flooring each leg independently before comparing sums, as the
    // non-DJINNI_FASTER path below does, could make an indirect sum look
    // shorter than the direct distance -- but that's an artifact of
    // rounding twice before comparing, not a real shorter path.)
    for (uint32_t i = 0; i < numCustomers; i++) {
      _timeMatrix[i].resize(numCustomers);
      for (uint32_t j = 0; j < numCustomers; j++)
        _timeMatrix[i][j] = ::floor(::sqrt(
            (_matrix[i][0] - _matrix[j][0]) * (_matrix[i][0] - _matrix[j][0]) +
            (_matrix[i][1] - _matrix[j][1]) * (_matrix[i][1] - _matrix[j][1])));
    }
#else
    for (uint32_t i = 0; i < numCustomers; i++) {
      _timeMatrix[i].resize(numCustomers);
      for (uint32_t j = 0; j < numCustomers; j++) {
        _timeMatrix[i][j] = ::sqrt(
            (_matrix[i][0] - _matrix[j][0]) * (_matrix[i][0] - _matrix[j][0]) +
            (_matrix[i][1] - _matrix[j][1]) * (_matrix[i][1] - _matrix[j][1]));
        _timeMatrix[i][j] = ::floor(_timeMatrix[i][j]);
      }
    }
    for (uint32_t i = 0; i < numCustomers; i++)
      for (uint32_t j = 0; j < numCustomers; j++)
        for (uint32_t k = 0; k < numCustomers; k++)
          if (_timeMatrix[i][j] > (_timeMatrix[i][k] + _timeMatrix[k][j]))
            _timeMatrix[i][j] = _timeMatrix[i][k] + _timeMatrix[k][j];
#endif
    _lowdeadlines.resize(numCustomers);
    _deadlines.resize(numCustomers);
    for (uint32_t i = 0; i < numCustomers; i++) {
#ifdef USE_BOUNDS_CHECKING
      _lowdeadlines.at(i) = _matrix[i][3];
      _deadlines.at(i) = _matrix[i][4];
#else
      _lowdeadlines[i] = _matrix[i][3];
      _deadlines[i] = _matrix[i][4];
#endif
    }
  }
};

//! A representation of information needed for the Traveling Salesman Problem.
/*! While many different WorldTypes can be used with TravelingSalesmanSolution,
   it has been most thoroughly tested with TravelingSalesmanWorld.  Attempting
   to use other world types may shake loose some interesting bugs.  Or they
   might not and our code could be perfect.  We don't know.  Don't panic, and
   have fun.

    @author Hansen, Thiede
    @since 2.0
*/
template <class WorldType> class TravelingSalesmanSolution {
public:
  /*! A constructor that uses an already initialized World object.
  @param w A WorldType object */
  explicit TravelingSalesmanSolution(const WorldType &w)
      : _w(new WorldType(w)), _f{0.0}, _p{0.0}, _firstswitch{0},
        _secondswitch{0}, _firstarrival{0}, _firstpenalty{0} {
    resizeToWorld();
  }

  /*! A constructor that initializes a new WorldType.

  @param worldParam A char* containing parameters used to initialize a new
  object of type WorldType */
  explicit TravelingSalesmanSolution(const char *worldParam)
      : _w(new WorldType{}), _f{0.0}, _p{0.0}, _firstswitch{0},
        _secondswitch{0}, _firstarrival{0}, _firstpenalty{0} {
    *_w = WorldType::loadFromDumasFile(worldParam);
    resizeToWorld();
  }

  /*! Virtualized for the benefit of future subclassing. */
  virtual ~TravelingSalesmanSolution() = default;

  /*! Sets the Feasible component of the solution
  @param f The new feasible component */
  void setF(const double &f) { _f = f; }

  /*! Sets the Penalty component of the solution
  @param p The new penalty component */
  void setP(const double &p) { _p = p; }

  /*! Returns the Feasible component of the solution
  @return The feasible component of the current solution */
  [[nodiscard]] double getF() const { return _f; }

  /*! Returns the Penalty component of the solution
  @return the Penalty component of the current solution */
  [[nodiscard]] double getP() const { return _p; }

  /*! Generates a neighbor TravelingSalesmanSolution from this current
  TravelingSalesmanSolution.
  @param neighbor The TravelingSalesmanSolution object which will receive the
  value.*/
  void generateNeighbor(TravelingSalesmanSolution &neighbor) {
    uint32_t firstswitch = 0;
    uint32_t numCustomers = _solution.size();
    neighbor.setF(getF());
    neighbor.setP(getP());
    while (0 == firstswitch)
      firstswitch = static_cast<uint32_t>((numCustomers - 1) * _dis(_prng)) + 1;
    uint32_t secondswitch = firstswitch;
    while ((secondswitch == firstswitch) || (secondswitch == firstswitch - 1))
      secondswitch =
          static_cast<uint32_t>((numCustomers - 1) * _dis(_prng)) + 1;
    uint32_t holder = _solution[firstswitch];
    if (firstswitch < secondswitch) {
      std::copy(_solution.begin(), _solution.begin() + firstswitch,
                neighbor._solution.begin());
      std::copy(_solution.begin() + firstswitch + 1,
                _solution.begin() + secondswitch + 1,
                neighbor._solution.begin() + firstswitch);
      neighbor._solution[secondswitch] = holder;
      neighbor._firstarrival =
          static_cast<uint32_t>(_arrivaltime[firstswitch - 1]);
      neighbor._firstpenalty =
          static_cast<uint32_t>(_penaltysum[firstswitch - 1]);
      std::copy(_arrivaltime.begin(), _arrivaltime.begin() + firstswitch,
                neighbor._arrivaltime.begin());
      std::copy(_penaltysum.begin(), _penaltysum.begin() + firstswitch,
                neighbor._penaltysum.begin());
    } else {
      std::copy(_solution.begin(), _solution.begin() + secondswitch + 1,
                neighbor._solution.begin());
      std::copy(_solution.begin() + secondswitch + 1,
                _solution.begin() + firstswitch,
                neighbor._solution.begin() + secondswitch + 2);
      neighbor._solution[secondswitch + 1] = holder;
      neighbor._firstarrival =
          static_cast<uint32_t>(_arrivaltime[secondswitch]);
      neighbor._firstpenalty = static_cast<uint32_t>(_penaltysum[secondswitch]);
      std::copy(_arrivaltime.begin(), _arrivaltime.begin() + secondswitch + 1,
                neighbor._arrivaltime.begin());
      std::copy(_penaltysum.begin(), _penaltysum.begin() + secondswitch + 1,
                neighbor._penaltysum.begin());
    }
    if (std::max(firstswitch, secondswitch) != numCustomers)
      std::copy(_solution.begin() + std::max(firstswitch, secondswitch) + 1,
                _solution.end(),
                neighbor._solution.begin() +
                    std::max(firstswitch, secondswitch) + 1);
    neighbor._firstswitch = firstswitch;
    neighbor._secondswitch = secondswitch;
    neighbor.update();
  }

protected:
  /*! Update schedules, member data, etc., based on current state.

  Not part of the public interface: it's only safe to call once
  generateNeighbor() has populated _firstswitch/_secondswitch, which is
  the only place this library calls it. Calling it earlier (e.g. right
  after construction, when both default to 0) reads _solution at index
  -1. */
  void update() {
    // firstswitch/secondswitch are always in [1, numCustomers-1]: only
    // generateNeighbor() sets them (see this function's docs above), and
    // its switch-index picker never produces 0. That invariant is what
    // makes every "- 1" below safe as unsigned arithmetic instead of the
    // signed scratch space this function used to hold them in.
    assert(_firstswitch >= 1 && _secondswitch >= 1 &&
           _firstswitch < _solution.size() && _secondswitch < _solution.size());
    double cost = getF();
    uint32_t firstswitch = _firstswitch;
    uint32_t secondswitch = _secondswitch;
    uint32_t numCustomers = _solution.size();
    const std::vector<uint32_t> &tour = _solution;
    const Matrix<double, 2> &travTime = _w->travelTimes();
    if (firstswitch <= secondswitch) {
      if (secondswitch != (numCustomers - 1)) {
        cost -= (travTime[tour[firstswitch - 1]][tour[secondswitch]] +
                 travTime[tour[secondswitch]][tour[firstswitch]] +
                 travTime[tour[secondswitch - 1]][tour[secondswitch + 1]]);
        cost += (travTime[tour[firstswitch - 1]][tour[firstswitch]] +
                 travTime[tour[secondswitch - 1]][tour[secondswitch]] +
                 travTime[tour[secondswitch]][tour[secondswitch + 1]]);
      } else {
        cost -= (travTime[tour[firstswitch - 1]][tour[secondswitch]] +
                 travTime[tour[secondswitch]][tour[firstswitch]] +
                 travTime[tour[secondswitch - 1]][tour[0]]);
        cost += (travTime[tour[firstswitch - 1]][tour[firstswitch]] +
                 travTime[tour[secondswitch - 1]][tour[secondswitch]] +
                 travTime[tour[secondswitch]][tour[0]]);
      }
      timingUpdate();
    } else {
      if (firstswitch != (numCustomers - 1)) {
        cost -= (travTime[tour[secondswitch]][tour[secondswitch + 2]] +
                 travTime[tour[firstswitch]][tour[secondswitch + 1]] +
                 travTime[tour[secondswitch + 1]][tour[firstswitch + 1]]);
        cost += (travTime[tour[firstswitch]][tour[firstswitch + 1]] +
                 travTime[tour[secondswitch]][tour[secondswitch + 1]] +
                 travTime[tour[secondswitch + 1]][tour[secondswitch + 2]]);
      } else {
        cost -= (travTime[tour[secondswitch]][tour[secondswitch + 2]] +
                 travTime[tour[firstswitch]][tour[secondswitch + 1]] +
                 travTime[tour[secondswitch + 1]][tour[0]]);
        cost += (travTime[tour[firstswitch]][tour[0]] +
                 travTime[tour[secondswitch]][tour[secondswitch + 1]] +
                 travTime[tour[secondswitch + 1]][tour[secondswitch + 2]]);
      }
      timingUpdate();
    }
    setF(cost);
    setP(_penaltysum[numCustomers - 1]);
  }

public:
  /*! Randomize this TravelingSalesmanSolution. */
  void randomize() {
    for (uint32_t i = 0; i < _solution.size(); i += 1)
      _solution[i] = i;
    auto iter = _solution.begin();
    ++iter;
    std::shuffle(iter, _solution.end(), _prng);
  }

  /*! Copy constructor.

  Deliberately does not copy _prng/_dis: the new object gets its own
  freshly-seeded engine (via their default member initializers) rather
  than starting in lockstep with route's. Annealer relies on this --
  _best/_current/_neighbor are all copy-constructed from one another, and
  if they shared or cloned RNG state, their "independent" random streams
  would actually be correlated.

  @param route The route to copy from. */
  TravelingSalesmanSolution(const TravelingSalesmanSolution<WorldType> &route)
      : _w(route._w), _solution(route._solution), _f(route._f), _p(route._p),
        _arrivaltime(route._arrivaltime), _penaltysum(route._penaltysum),
        _firstswitch(route._firstswitch), _secondswitch(route._secondswitch),
        _firstarrival(route._firstarrival), _firstpenalty(route._firstpenalty) {
  }

  /*! Copy assignment operator.

  Like the copy constructor, deliberately leaves _prng/_dis alone: this
  object keeps generating from its own independent stream across the
  assignment, rather than adopting route's.

  @param route The route to copy from.
  @return *this, for chaining. */
  TravelingSalesmanSolution &
  operator=(const TravelingSalesmanSolution<WorldType> &route) {
    if (this == &route)
      return *this;
    _w = route._w;
    _solution = route._solution;
    _f = route._f;
    _p = route._p;
    _arrivaltime = route._arrivaltime;
    _penaltysum = route._penaltysum;
    _firstswitch = route._firstswitch;
    _secondswitch = route._secondswitch;
    _firstarrival = route._firstarrival;
    _firstpenalty = route._firstpenalty;
    return *this;
  }

  /*! Move constructor.

  Unlike the copy constructor, this does move _prng/_dis: route is being
  discarded, not kept alive alongside this object, so there's no
  correlated-streams risk to guard against here -- taking over its
  already-seeded engine is just cheaper than reseeding from scratch. This
  exists so Annealer can hold SolutionType by value and swap _current/
  _neighbor via std::swap() in O(1) instead of copying the full solution
  on every accepted move.

  @param route The route to move from. */
  TravelingSalesmanSolution(
      TravelingSalesmanSolution<WorldType> &&route) noexcept
      : _w(std::move(route._w)), _solution(std::move(route._solution)),
        _f(route._f), _p(route._p), _arrivaltime(std::move(route._arrivaltime)),
        _penaltysum(std::move(route._penaltysum)),
        _firstswitch(route._firstswitch), _secondswitch(route._secondswitch),
        _firstarrival(route._firstarrival), _firstpenalty(route._firstpenalty),
        _prng(std::move(route._prng)), _dis(std::move(route._dis)) {}

  /*! Move assignment operator.

  Like the move constructor (and unlike copy assignment), this moves
  _prng/_dis too -- see the move constructor's docs for why.

  @param route The route to move from.
  @return *this, for chaining. */
  TravelingSalesmanSolution &
  operator=(TravelingSalesmanSolution<WorldType> &&route) noexcept {
    if (this == &route)
      return *this;
    _w = std::move(route._w);
    _solution = std::move(route._solution);
    _f = route._f;
    _p = route._p;
    _arrivaltime = std::move(route._arrivaltime);
    _penaltysum = std::move(route._penaltysum);
    _firstswitch = route._firstswitch;
    _secondswitch = route._secondswitch;
    _firstarrival = route._firstarrival;
    _firstpenalty = route._firstpenalty;
    _prng = std::move(route._prng);
    _dis = std::move(route._dis);
    return *this;
  }

  /*! Dump our current path to an output stream.
  @param os The output stream to dump our path to
  @return The output stream after we've dumped in it */
  std::ostream &dump(std::ostream &os) const {
    std::ostream_iterator<uint32_t> oiter(os, " ");
    std::ranges::copy(_solution, oiter);
    return os;
  }

  /*! Computes the feasible and penalty portions of this
   * TravelingSalesmanSolution. */
  void compute() {
    double addEnergy = 0;
    double energy = 0;
    double minutesMissed = 0;
    double routeTime = 0;

    const Matrix<double, 2> &travTime = _w->travelTimes();
    const std::vector<double> &lowdeadlines = _w->lowDeadlines();
    const std::vector<double> &deadlines = _w->deadlines();

    _penaltysum[0] = 0;
    _arrivaltime[0] = 0;

    for (uint32_t i = 0; i < _solution.size() - 1; ++i) {
      energy += travTime[_solution[i]][_solution[i + 1]];
      routeTime += travTime[_solution[i]][_solution[i + 1]];
      _arrivaltime[i + 1] = energy;
      if (energy < lowdeadlines[_solution[i + 1]]) {
        addEnergy = lowdeadlines[_solution[i + 1]] - energy;
        energy += addEnergy;
      }
      if (energy > deadlines[_solution[i + 1]])
        minutesMissed += energy - deadlines[_solution[i + 1]];
      _penaltysum[i + 1] = minutesMissed;
    }
    //        energy += travTime[_solution[_solution.size() - 1]][_solution[0]];
    routeTime += travTime[_solution[_solution.size() - 1]][_solution[0]];
    setF(routeTime);
    setP(minutesMissed);
  }

protected:
  //! The fewest customers generateNeighbor()'s 2-opt index picker can
  //! guarantee a legal (in-bounds, terminating) pair of switch points for.
  //! Below this, either the picked index falls outside _solution (as low
  //! as one customer) or no valid secondswitch remains for some draws of
  //! firstswitch (two or three customers), hanging the rejection loop.
  static constexpr uint32_t kMinCustomers = 4;

  /*! Sizes _solution/_arrivaltime/_penaltysum to match the loaded world.

  @throws std::invalid_argument if the world has fewer than kMinCustomers
  customers, since generateNeighbor() cannot safely pick switch points
  below that size. */
  void resizeToWorld() {
    if (_w->data().size() < kMinCustomers)
      throw std::invalid_argument(
          "TravelingSalesmanSolution requires at least " +
          std::to_string(kMinCustomers) + " customers");
    _solution.resize(_w->data().size(), 0);
    _arrivaltime.resize(_solution.size(), 0);
    _penaltysum.resize(_solution.size(), 0);
  }

  /*! Update the travel schedule. */
  void timingUpdate() {
    // See update()'s assert above: same invariant, same reason it's safe
    // to use unsigned arithmetic for start/i below.
    assert(_firstswitch >= 1 && _secondswitch >= 1 &&
           _firstswitch < _solution.size() && _secondswitch < _solution.size());
    uint32_t start;
    uint32_t numCustomers = _solution.size();
    const Matrix<double, 2> &travTime = _w->travelTimes();
    const std::vector<double> &lowdeadlines = _w->lowDeadlines();
    const std::vector<double> &deadlines = _w->deadlines();
    const std::vector<uint32_t> &tour = _solution;

    if (_firstswitch < _secondswitch)
      start = _firstswitch;
    else
      start = _secondswitch + 1;

    _arrivaltime[start - 1] = _firstarrival;
    _penaltysum[start - 1] = _firstpenalty;
    for (uint32_t i = start; i <= numCustomers - 1; i++) {
      if (_arrivaltime[i - 1] >= lowdeadlines[tour[i - 1]])
        _arrivaltime[i] = _arrivaltime[i - 1] + travTime[tour[i - 1]][tour[i]];
      else
        _arrivaltime[i] =
            lowdeadlines[tour[i - 1]] + travTime[tour[i - 1]][tour[i]];
      if (_arrivaltime[i] > deadlines[tour[i]])
        _penaltysum[i] =
            _penaltysum[i - 1] + (_arrivaltime[i] - deadlines[tour[i]]);
      else
        _penaltysum[i] = _penaltysum[i - 1];
    }
  }

  std::shared_ptr<WorldType> _w;
  std::vector<uint32_t> _solution;
  double _f, _p;
  std::vector<double> _arrivaltime;
  std::vector<double> _penaltysum;
  uint32_t _firstswitch, _secondswitch, _firstarrival, _firstpenalty;

  // Per-instance, not static: see the copy constructor and copy
  // assignment operator above for why sharing (or even cloning) this
  // across instances would be wrong, not just a missed optimization.
  std::mt19937_64 _prng{std::random_device{}()};
  std::uniform_real_distribution<> _dis{0.0, 1.0};
};

/*! An operator<< overloaded for TravelingSalesmanSolution.

    @param os An output stream to write to
    @param sol A TravelingSalesmanSolution to write
    @return An output stream after we've written to it
*/
template <class WorldType>
std::ostream &operator<<(std::ostream &os,
                         const TravelingSalesmanSolution<WorldType> &sol) {
  return sol.dump(os);
}

/*! A more human-readable version of a fully qualified typename. */
typedef TravelingSalesmanSolution<TravelingSalesmanWorld> TravelingSalesman;
} // namespace edu::uiowa::tippie::djinni
