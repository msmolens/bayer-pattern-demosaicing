/*
 * Max Smolens
 * max@cs.unc.edu
 */

#include <iostream>
#include "BayerRendererCPU.hpp"
#include "bayer.h"

BayerRendererCPU::BayerRendererCPU () : width (0),
					height (0)
{
}

BayerRendererCPU::~BayerRendererCPU ()
{
	if (rgb != NULL) {
		delete [] rgb;
	}
	glDeleteTextures (1, &tex);
}

bool
BayerRendererCPU::Initialize ()
{
	// Check for required extensions
	if (!GLEW_NV_texture_rectangle) {
		std::cerr << "ERROR: GL_NV_texture_rectangle not supported" << std::endl;
		return false;
	}

	// Set up textures
	InitializeTextures ();

	rgb = new GLubyte[3 * width * height];
	
	return true;
}

bool
BayerRendererCPU::Initialize (int w,
			      int h)
{
	width = w;
	height = h;
	
	return Initialize ();
}

void
BayerRendererCPU::SetBayer (const GLubyte *bayer) const {
	gp_bayer_decode (bayer, width, height, rgb, BAYER_TILE_GRBG);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex);
	glTexSubImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, rgb);
}

void
BayerRendererCPU::Bind () const
{
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex);
}

void
BayerRendererCPU::EnableTextureTarget () const
{
	glEnable (GL_TEXTURE_RECTANGLE_NV);
}

void
BayerRendererCPU::DisableTextureTarget () const
{
	glDisable (GL_TEXTURE_RECTANGLE_NV);
}

void
BayerRendererCPU::InitializeTextures ()
{
	glGenTextures (1, &tex);

	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}
