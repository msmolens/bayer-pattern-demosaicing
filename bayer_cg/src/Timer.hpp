/*
	Max Smolens
	max@cs.unc.edu

	USAGE:
	Timer timer;
	while (loop) {
		timer.start ();
		// do some operation
		if (timer.stop ()) {
			// get average time in seconds of operation over timer.get_limit () times
			double average_time = timer.get_average_time ();
		}
	}
*/

#ifndef TIMER_HPP
#define TIMER_HPP

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

class Timer
{
public:
	Timer ();
	~Timer ();
	void start ();
	bool stop ();
	double get_average_time ();
	unsigned int get_limit () const;
	void set_limit (unsigned int limit);

private:
	void reset ();

private:
	double time;
	unsigned int count;
	unsigned int limit;
#ifdef _WIN32
	LARGE_INTEGER elapsed_time;
	LARGE_INTEGER timer_start;
	LARGE_INTEGER timer_end;
	LARGE_INTEGER timer_freq;
#else
	struct timeval elapsed_time;
	struct timeval timer_start;
	struct timeval timer_end;
#endif
};

inline void
Timer::set_limit (unsigned int limit)
{
	this->limit = limit;
}

inline unsigned int
Timer::get_limit () const
{
	return limit;
}

inline void
Timer::start ()
{
#ifdef _WIN32
	QueryPerformanceCounter (&timer_start);
#else
	gettimeofday (&timer_start, NULL);
#endif
}

inline bool
Timer::stop ()
{
#ifdef _WIN32
	QueryPerformanceCounter (&timer_end);
	elapsed_time.QuadPart += timer_end.QuadPart - timer_start.QuadPart;
#else
	gettimeofday (&timer_end, NULL);
	elapsed_time.tv_sec += timer_end.tv_sec - timer_start.tv_sec;
	elapsed_time.tv_usec += timer_end.tv_usec - timer_start.tv_usec;
	time += elapsed_time.tv_sec + elapsed_time.tv_usec / 1.0e6;
#endif
	count ++;
	if (count >= limit) {
		return true;
	}
	return false;
}

inline double
Timer::get_average_time ()
{
	// seconds
#ifdef _WIN32
	double time = (double)elapsed_time.QuadPart/((double)timer_freq.QuadPart*(double)count);
#else
//	double time = elapsed_time.tv_sec + elapsed_time.tv_usec / 1.0e6;
//	time /= limit;
#endif
	time /= limit;
	if (time < 0.0) time = 0.0;
	double t = time;
	reset ();
	return t;
}

inline void
Timer::reset ()
{
	time = 0;
	count = 0;
#ifdef _WIN32
	elapsed_time.QuadPart = 0;
#else
	elapsed_time.tv_sec = 0;
	elapsed_time.tv_usec = 0;
#endif
}

#endif // TIMER_HPP
