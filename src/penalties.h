/* Penalties.h is (c) 2006, Robert J. Hansen.
 *
 * This code may be used, modified and/or redistributed under the terms
 * of the modified BSD license.
 */

#ifndef PENALTIES_H
#define PENALTIES_H

#include <cmath>

//! A penalty function for annealing which substantially implements the Ohlmann-Thomas compressed annealing functionality.

/*! Unfortunately, this class does not entirely encapsulate the necessary functionality.  A trivial specialization of the Annealer class is required.  Interested parties are referred to the Annealer.initializeParam() method for more details.

    @author Hansen, Thiede
    @since 2.1
*/
class Compressed {
    public:
    
    /*! All PenaltyFuncs must typedef their return value as ReturnType.
    
    For compressed annealing, the lambda value is represented as a double.
    Hence, we typedef double to ReturnType. */
    typedef double ReturnType;

    /*! Since each PenaltyFunc must declare the type of its ReturnType,
    it must also be able to create a default value of that type so that
    an Annealer can initialize it to the correct value. */
    constexpr static double defaultReturnTypeValue = 0.0;
    
    /*! Default constructor.
    
    Please note that this will create a Compressed object with values you
    probably won't like.  Make sure to set all values appropriately before
    use. */
    Compressed() {}
    
    /*! A constructor which initializes all data members at once.
    
    This constructor is the one you should probably be using.
    
    @param expPower The exponential factor involved in compression
    @param pcap The pressure cap
    @param cperc The percentage of cap
    */
    Compressed(double expPower, double pcap, double cperc) : 
    	_expPower(expPower), _pressureCap(pcap), _capPercentage(cperc)
    {
    }

    /*! This copy constructor should be entirely unnecessary.
    
    @param pfun The Compressed object to be copied */
    Compressed(const Compressed& pfun) : 
    	_expPower(pfun._expPower), _pressureCap(pfun._pressureCap), _capPercentage(pfun._capPercentage)
    {
    }

    /*! A no-op destructor.
    
    This destructor has been virtualized in case you wish to later subclass off Compressed. */
    virtual ~Compressed()
    {
    }

    /*! Sets the exponential factor for the compression.
    
    @param power The new exponential power to use */
    void setPower(const double power) 
    { 
    	_expPower = power;
    }
    
    /*! Sets the pressure cap.
    
    @param cap The new pressure cap to use */
    void setPressureCap(const double cap) 
    { 
    	_pressureCap = cap; 
    }

    /*! Sets the percentage of cap to use.
    
    @param perc The percentage of cap to use. */    
    void setCapPercentage(const double perc)
    {
        _capPercentage = perc;
    }
    
    /*! Gets the cap percentage.
    
    @return The cap percentage */
    double getCapPercentage() const
    {
        return _capPercentage;
    }
    
    /*! Gets the current exponential factor used in compression.
    
    @return The exponential factor used in compression */
    double getExpPower() const
    {
        return _expPower;
    }
    
    /*! The required operator()(int) common to all PenaltyFuncs.
    
    In compressed annealing, the return value varies over iterations.
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
/*! @warning This class has not been tested.  Please do not rely on its correct operation.
    @author Hansen, Thiede
    @since 2.1
*/
class Simulated {
	public:
	
	/*! Like compressed annealing, simulated annealing uses doubles for its lambda. */
	typedef double ReturnType;
	constexpr static double defaultReturnTypeValue = 0.0;

	
	/*! A convenience constructor which initializes the multiplier to 1.0. */
	Simulated() : _mult(1.0) 
	{
	}
	
	/*! A constructor that sets the multiplier for use in simulated annealing.
	
	@param multiplier The multiplier to use */
	Simulated(const double multiplier) : _mult(multiplier) 
	{
	}

    /*! This copy constructor should be unnecessary.  It's included in the event end-users feel like getting funky.
    
    @param sim The Simulated object to be copied
    */	
	Simulated(const Simulated& sim) : _mult(sim._mult) 
	{
	}
	
	/*! A no-op destructor.
	
	This destructor has been virtualized in case end-users wish to subclass. */
	virtual ~Simulated() 
	{
	}
	
	/*! Sets the multiplier for use in simulated annealing.
	
	@param multiplier The multiplier to use */
	void setMultiplier(const double multiplier) 
	{ 
		_mult = multiplier; 
	}
	
	/*! All PenaltyFuncs must implement operator()(const int iter).
	
	However, for simulated annealing a constant value is always returned.

	*/
	ReturnType operator()(const int) const 
	{
		return _mult; 
	}

	protected:
	double _mult;
};


#endif
