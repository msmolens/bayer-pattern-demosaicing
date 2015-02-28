#ifndef IMAGE_HPP
#define IMAGE_HPP

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

// Reads image file into memory.
GLubyte *
read_image (const char *filename,
	    int &width,
	    int &height);

/// Writes data to RGB image file.
int
write_image (const char *filename,
	     const unsigned char *data,
	     int width,
	     int height);

#endif // IMAGE_HPP
