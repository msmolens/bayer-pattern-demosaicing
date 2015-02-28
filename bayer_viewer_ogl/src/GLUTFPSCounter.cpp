#include <GL/glut.h>
#include "GLUTFPSCounter.hpp"

GLUTFPSCounter::GLUTFPSCounter () :
	running (false),
	frame (0) {
}

GLUTFPSCounter::~GLUTFPSCounter ()
{
}

void
GLUTFPSCounter::start ()
{
	running = true;
	frame = 0;
	begin_time = glutGet (GLUT_ELAPSED_TIME);
}

void
GLUTFPSCounter::stop ()
{
	running = false;
}

bool
GLUTFPSCounter::update ()
{
	float delta;
	
	if (running) {
		frame++;
		int now = glutGet (GLUT_ELAPSED_TIME);
		if ((delta = (now - begin_time)) >= INTERVAL) {
			fps = static_cast<float>(frame)*1000.0f/delta;
			begin_time = now;
			frame = 0;
			return true;
		}
	}
	return false;
}
