/**
 * @file   FPSCounter.hpp
 * @author Max Smolens
 * @brief  Frames per second (FPS) counter.
 */

#ifndef FPSCOUNTER_HPP
#define FPSCOUNTER_HPP

/**
 * Frames per second (FPS) counter.  Computes the FPS of an OpenGL application.
 */
class FPSCounter
{
public:
	/**
	 * Constructor.
	 */
	FPSCounter ();

	/**
	 * Destructor.
	 */
	virtual ~FPSCounter ()=0;

	/** 
	 * Starts the frame rate timer and resets the frame counter.
	 */
	virtual void start ()=0;

	/** 
	 * Stops the frame rate timer.
	 */
	virtual void stop ()=0;
	
	/**
	 * Call at start of display loop to update the FPS count.
	 */
	virtual bool update ()=0;

	/**
	 * Returns the FPS over the last interval of #INTERVAL.
	 */
	virtual float get_fps () const;

public:
	/// Minimum number of milliseconds to integrate framerate over.
	static const int INTERVAL;
	
protected:
	/// FPS over the last interval.
	float fps;
};

inline float
FPSCounter::get_fps () const
{
	return fps;
}

#endif // FPSCOUNTER_HPP
