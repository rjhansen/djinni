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

#ifndef PENALTIES_H
#define PENALTIES_H

#include <cmath>

//! A penalty function for annealing which substantially implements the
//! Ohlmann-Thomas Compression annealing functionality.

/*! Unfortunately, this class does not entirely encapsulate the necessary
   functionality.  A trivial specialization of the Annealer class is required.
   Interested parties are referred to the Annealer.initializeParam() method for
   more details.

    @author Hansen, Thiede
    @since 2.1
*/
class Compression {
public:
    /*! All PenaltyFuncs must typedef their return value as ReturnType.

    For Compression annealing, the lambda value is represented as a double.
    Hence, we typedef double to ReturnType. */
    typedef double ReturnType;

    /*! Since each PenaltyFunc must declare the type of its ReturnType,
    it must also be able to create a default value of that type so that
    an Annealer can initialize it to the correct value. */
    constexpr static double defaultReturnTypeValue = 0.0;

    /*! Default constructor.

    Please note that this will create a Compression object with values you
    probably won't like.  Make sure to set all values appropriately before
    use. */
    Compression():
    _expPower(0),
    _pressureCap(0),
    _capPercentage(0) {
    }

    /*! A constructor which initializes all data members at once.

    This constructor is the one you should probably be using.

    @param expPower The exponential factor involved in compression
    @param pcap The pressure cap
    @param cperc The percentage of cap
    */
    Compression(double expPower, double pcap, double cperc)
        : _expPower(expPower)
        , _pressureCap(pcap)
        , _capPercentage(cperc)
    {
    }

    /*! This copy constructor should be entirely unnecessary.

    @param pfun The Compression object to be copied */
    Compression(const Compression& pfun)
        : _expPower(pfun._expPower)
        , _pressureCap(pfun._pressureCap)
        , _capPercentage(pfun._capPercentage)
    {
    }

    /*! A no-op destructor.

    This destructor has been virtualized in case you wish to later subclass off
    Compression. */
    virtual ~Compression() { }

    /*! Sets the exponential factor for the compression.

    @param power The new exponential power to use */
    void setPower(const double power) { _expPower = power; }

    /*! Sets the pressure cap.

    @param cap The new pressure cap to use */
    void setPressureCap(const double cap) { _pressureCap = cap; }

    /*! Sets the percentage of cap to use.

    @param perc The percentage of cap to use. */
    void setCapPercentage(const double perc) { _capPercentage = perc; }

    /*! Gets the cap percentage.

    @return The cap percentage */
    double getCapPercentage() const { return _capPercentage; }

    /*! Gets the current exponential factor used in compression.

    @return The exponential factor used in compression */
    double getExpPower() const { return _expPower; }

    /*! The required operator()(int) common to all PenaltyFuncs.

    In Compression annealing, the return value varies over iterations.
    For many other types, this will simply return a constant value. */
    ReturnType operator()(const int iter) const
    {
        return _pressureCap * (1 - exp(-1 * _expPower * iter));
    }

protected:
    double _expPower;
    double _pressureCap;
    double _capPercentage;
};

//! A class representing classical simulated annealing.
/*! @warning This class has not been tested.  Please do not rely on its correct
   operation.
    @author Hansen, Thiede
    @since 2.1
*/
class Simulated {
public:
    /*! Like Compression annealing, simulated annealing uses doubles for its
     * lambda. */
    typedef double ReturnType;
    constexpr static double defaultReturnTypeValue = 0.0;

    /*! A convenience constructor which initializes the multiplier to 1.0. */
    Simulated()
        : _mult(1.0)
    {
    }

    /*! A constructor that sets the multiplier for use in simulated annealing.

    @param multiplier The multiplier to use */
    Simulated(const double multiplier)
        : _mult(multiplier)
    {
    }

    /*! This copy constructor should be unnecessary.  It's included in the event
    end-users feel like getting funky.

    @param sim The Simulated object to be copied
    */
    Simulated(const Simulated& sim)
        : _mult(sim._mult)
    {
    }

    /*! A no-op destructor.

    This destructor has been virtualized in case end-users wish to subclass. */
    virtual ~Simulated() { }

    /*! Sets the multiplier for use in simulated annealing.

    @param multiplier The multiplier to use */
    void setMultiplier(const double multiplier) { _mult = multiplier; }

    /*! All PenaltyFuncs must implement operator()(const int iter).

    However, for simulated annealing a constant value is always returned.

    */
    ReturnType operator()(const int) const { return _mult; }

protected:
    double _mult;
};

#endif
