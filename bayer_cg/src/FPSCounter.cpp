#include "FPSCounter.hpp"

const int FPSCounter::INTERVAL = 1000;

FPSCounter::FPSCounter () :
	fps (0.0)
{
}

FPSCounter::~FPSCounter ()
{
}
