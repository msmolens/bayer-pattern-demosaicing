#include "Timer.hpp"

Timer::Timer () :
	limit (100)
{
#ifdef _WIN32
	QueryPerformanceFrequency (&timer_freq);
#endif
	reset ();
}

Timer::~Timer ()
{
}
