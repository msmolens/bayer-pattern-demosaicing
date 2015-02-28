/**
 * @file   GLUTFPSCounter.hpp
 * @author Max Smolens
 * @brief  GLUT frames per second (FPS) counter.
 *
 * Based on GLVU's FPS counter.
 */

#ifndef GLUTFPSCOUNTER_HPP
#define GLUTFPSCOUNTER_HPP

#include <FPSCounter.hpp>

/**
 * Frames per second (FPS) counter.  Computes the FPS of an OpenGL application.
 */
class GLUTFPSCounter : public FPSCounter
{
public:
	/**
	 * Constructor.
	 */
	GLUTFPSCounter ();

	/**
	 * Destructor.
	 */
	virtual ~GLUTFPSCounter ();

	/** 
	 * Starts the frame rate timer and resets the frame counter.
	 */
	void start ();

	/** 
	 * Stops the frame rate timer.
	 */
	void stop ();
	
	/**
	 * Call at start of display loop to update the FPS count.
	 */
	bool update ();
	
private:
	/// True if frame rate timer is running.
	bool running;
	
	/// Current frame number.
	int frame;

	/// Time at start of current interval.
	int begin_time;
};

#endif // GLUTFPSCOUNTER_HPP
