#ifndef ROUTES_H
#define ROUTES_H

#include "utils.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

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
  Matrix() {}
  Matrix(std::vector<Matrix<T, N - 1>> matrix) : _matrix(matrix) {}
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
  uint32_t dimensions() const { return Matrix<T, N>::DIMENSIONS; }
  void reset() {
    for (uint32_t i = 0; i < _matrix.size(); i += 1)
      _matrix[i].reset();
  }
  uint32_t size() const { return _matrix.size(); }
  void push_back(Matrix<T, N - 1> matrix) { _matrix.push_back(matrix); }
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
  Matrix(std::vector<T> vec) : _matrix(vec) {}
#ifdef USE_BOUNDS_CHECKING
  const T &operator[](const uint32_t n) const { return _matrix.at(n); }
  T &operator[](const uint32_t n) { return _matrix.at(n); }
#else
  const T &operator[](const uint32_t n) const { return _matrix[n]; }
  T &operator[](const uint32_t n) { return _matrix[n]; }
#endif
  uint32_t dimensions() const { return Matrix<T, 1>::DIMENSIONS; }
  void reset() { _matrix.clear(); }
  uint32_t size() const { return _matrix.size(); }
  void push_back(std::vector<T> vec) { _matrix.push_back(vec); }
  void resize(const uint32_t n) { _matrix.resize(n, T()); }

protected:
  std::vector<T> _matrix;
  static const uint32_t DIMENSIONS = 1;
};

//! A class representing an instance of the Traveling Salesman Problem with Time
//! Windows.

/*! @author Hansen, Ohlmann, Thomas
    @since 1.0
*/
class TSPTWWorld {
public:
  TSPTWWorld(const std::string &filename);
  TSPTWWorld(const char *filename);
  virtual ~TSPTWWorld() {}

  const Matrix<double, 2> &travelTimes() const { return _timeMatrix; }
  const std::vector<double> &lowDeadlines() const { return _lowdeadlines; }
  const std::vector<double> &deadlines() const { return _deadlines; }

  //! Returns a const-reference to the Matrix used to store this world's data.
  const Matrix<double, 2> &data() const { return _matrix; }

  //! Returns a reference to the Matrix used to store this world's data.
  Matrix<double, 2> &data() { return _matrix; }

  //! Returns a const reference to the identifying string used for this World.
  const std::string &identifier() const { return _identifier; }

protected:
  Matrix<double, 2> _timeMatrix;
  Matrix<double, 2> _matrix;
  std::vector<double> _lowdeadlines, _deadlines;
  std::string _identifier;

  virtual void computeTravelTimes() {
    uint32_t numCustomers = _matrix.size();
    _timeMatrix.resize(numCustomers);
    for (uint32_t i = 0; i <= numCustomers - 1; i++) {
      _timeMatrix[i].resize(numCustomers);
      for (uint32_t j = 0; j <= numCustomers - 1; j++) {
        _timeMatrix[i][j] = ::sqrt(
            (_matrix[i][0] - _matrix[j][0]) * (_matrix[i][0] - _matrix[j][0]) +
            (_matrix[i][1] - _matrix[j][1]) * (_matrix[i][1] - _matrix[j][1]));
        _timeMatrix[i][j] = ::floor(_timeMatrix[i][j]);
      }
    }
    for (uint32_t i = 0; i <= numCustomers - 1; i++)
      for (uint32_t j = 0; j <= numCustomers - 1; j++)
        for (uint32_t k = 0; k <= numCustomers - 1; k++)
          if (_timeMatrix[i][j] > (_timeMatrix[i][k] + _timeMatrix[k][j]))
            _timeMatrix[i][j] = _timeMatrix[i][k] + _timeMatrix[k][j];
    _lowdeadlines.resize(numCustomers);
    _deadlines.resize(numCustomers);
    for (uint32_t i = 0; i <= numCustomers - 1; i++) {
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

std::ostream &operator<<(std::ostream &os, const TSPTWWorld &w);

//! A representation of information needed for the Traveling Salesman Problem.
/*! While many different WorldTypes can be used with TSPRoute, it has been most
    thoroughly tested with TSPTWWorld.  Attempting to use other world types may
        shake loose some interesting bugs.  Or they might not and our code could
be perfect.  We don't know.  Don't panic, and have fun.

@author Hansen, Thiede
@since 2.0
*/
template <class WorldType> class TSPRoute {
public:
  /*! A constructor that uses an already initialized World object.
  @param w A WorldType object */
  TSPRoute(WorldType &w)
      : _w(new WorldType(w)), _time(0.0), _cost(0.0), _timeWait(0.0) {
    _solution.resize(_w->data().size(), 0);
    _arrivaltime.resize(_solution.size(), 0);
    _penaltysum.resize(_solution.size(), 0);
    _identifier = "TSPRoute";
  }

  /*! A constructor that initializes a new WorldType.

  @param worldParam A char* containing parameters used to initialize a new
  object of type WorldType */
  TSPRoute(const char *worldParam)
      : _w(new WorldType(worldParam)), _time(0.0), _cost(0.0), _timeWait(0.0) {
    _solution.resize(_w->data().size(), 0);
    _arrivaltime.resize(_solution.size(), 0);
    _penaltysum.resize(_solution.size(), 0);
    _identifier = "TSPRoute";
  }

  /*! Virtualized for the benefit of future subclassing. */
  virtual ~TSPRoute() {}

  /*! Sets the Feasible component of the solution
  @param f The new feasible component */
  void setF(const double &f) { _f = f; }

  /*! Sets the Penalty component of the solution
  @param p The new penalty component */
  void setP(const double &p) { _p = p; }

  /*! Returns the Feasible component of the solution
  @return The feasible component of the current solution */
  double getF() const { return _f; }

  /*! Returns the Penalty component of the solution
  @return the Penalty component of the current solution */
  double getP() const { return _p; }

  /*! Generates a neighbor TSPRoute from this current TSPRoute.
  @param neighbor The TSPRoute object which will receive the value.*/
  void generateNeighbor(TSPRoute &neighbor) {
    uint32_t firstswitch = 0;
    uint32_t secondswitch;
    int holder;
    uint32_t numCustomers = _solution.size();
    neighbor.setF(getF());
    neighbor.setP(getP());
    while (0 == firstswitch)
      firstswitch = static_cast<int>((numCustomers - 1) * randomReal()) + 1;
    secondswitch = firstswitch;
    while ((secondswitch == firstswitch) || (secondswitch == firstswitch - 1))
      secondswitch = static_cast<int>((numCustomers - 1) * randomReal()) + 1;
    holder = _solution[firstswitch];
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

  /*! Update schedules, member data, etc, based on current state. */
  void update() {
    double cost = getF();
    int firstswitch = _firstswitch;
    int secondswitch = _secondswitch;
    int numCustomers = _solution.size();
    const std::vector<int> &tour = _solution;
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

  /*! Randomize this TSPRoute. */
  void randomize() {
    for (uint32_t i = 0; i < _solution.size(); i += 1)
      _solution[i] = static_cast<int>(i);
    std::vector<int>::iterator iter = _solution.begin();
    ++iter;
    std::shuffle(iter, _solution.end(), prng);
  }

  /*! Copy constructor.
  @param route The route to copy from. */
  TSPRoute(const TSPRoute<WorldType> &route)
      : _w(route._w), _solution(route._solution), _f(route._f), _p(route._p),
        _identifier(route._identifier), _arrivaltime(route._arrivaltime),
        _penaltysum(route._penaltysum), _time(route._time), _cost(route._cost),
        _timeWait(route._timeWait), _firstswitch(route._firstswitch),
        _secondswitch(route._secondswitch), _firstarrival(route._firstarrival),
        _firstpenalty(route._firstpenalty) {}

  /*! Dump our current path to an output stream.
  @param os The output stream to dump our path to
  @return The output stream after we've dumped in it */
  std::ostream &dump(std::ostream &os) const {
    std::ostream_iterator<int> oiter(os, " ");
    std::copy(_solution.begin(), _solution.end(), oiter);
    return os;
  }

  /*! Computes the feasible and penalty portions of this TSPRoute. */
  void compute() {
    double addEnergy = 0;
    double energy = 0;
    double minutesMissed = 0;
    double routeTime = 0;
    double waitTime = 0;

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
        waitTime += addEnergy;
        energy += addEnergy;
      }
      if (energy > deadlines[_solution[i + 1]])
        minutesMissed += energy - deadlines[_solution[i + 1]];
      _penaltysum[i + 1] = minutesMissed;
    }
    energy += travTime[_solution[_solution.size() - 1]][_solution[0]];
    routeTime += travTime[_solution[_solution.size() - 1]][_solution[0]];
    setF(routeTime);
    _time = routeTime;
    setP(minutesMissed);
    _timeWait = waitTime;
  }

protected:
  /*! Update the travel schedule. */
  void timingUpdate() {
    int start;
    int numCustomers = _solution.size();
    const Matrix<double, 2> &travTime = _w->travelTimes();
    const std::vector<double> &lowdeadlines = _w->lowDeadlines();
    const std::vector<double> &deadlines = _w->deadlines();
    const std::vector<int> &tour = _solution;

    if (_firstswitch < _secondswitch)
      start = _firstswitch;
    else
      start = _secondswitch + 1;

    _arrivaltime[start - 1] = _firstarrival;
    _penaltysum[start - 1] = _firstpenalty;
    for (int i = start; i <= numCustomers - 1; i++) {
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
  std::vector<int> _solution;
  double _f, _p;
  std::string _identifier;
  std::vector<double> _arrivaltime;
  std::vector<double> _penaltysum;
  double _time, _cost, _timeWait;
  uint32_t _firstswitch, _secondswitch, _firstarrival, _firstpenalty;
};

/*! An operator<< overloaded for TSPRoute.

    @param os An output stream to write to
    @param sol A TSPRoute to write
    @return An output stream after we've written to it
*/
template <class WorldType>
std::ostream &operator<<(std::ostream &os, const TSPRoute<WorldType> &sol) {
  return sol.dump(os);
}

/*! A more human-readable version of a fully qualified typename. */
typedef TSPRoute<TSPTWWorld> TravelingSalesman;

#endif
