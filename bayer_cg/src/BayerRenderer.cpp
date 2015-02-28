#include <iostream>
#include "RenderTexture.h"
#include "BayerRenderer.hpp"

const char *BayerRenderer::RENDERTEXTURE_INIT = "rgb texRECT";

const GLfloat BayerRenderer::MASK[] =
{
	// G1 B R G2
	0, 1.0, 0, 0,  0, 0, 1.0, 0,
	1.0, 0, 0, 0,  0, 0, 0, 1.0
};

BayerRenderer::BayerRenderer () : width (0),
				  height (0)
{
}

BayerRenderer::~BayerRenderer ()
{
	if (rt != NULL) {
		delete rt;
	}
	glDeleteTextures (1, &tex_bayer);
	glDeleteTextures (1, &tex_mask);
}

bool
BayerRenderer::Initialize (CGcontext context,
			   CGprofile vertexProfile,
			   CGprofile fragmentProfile)
{
	// Check for required extensions.
	if (!GLEW_NV_texture_rectangle) {
		std::cerr << "ERROR: GL_NV_texture_rectangle not supported" << std::endl;
		return false;
	}
	// TODO check for vertex/fragment program extensions

	// Register callback function for Cg errors and create an initial context.
//	cgSetErrorCallback (HandleCgError);
//	context = cgCreateContext ();

	this->context = context;
	this->vertexProfile = vertexProfile;
	this->fragmentProfile = fragmentProfile;
	LoadCgPrograms ();

	// Create RenderTexture.
	rt = CreateRenderTexture (width, height);

	// Set up bilinear interpolation.
	rt->Bind ();

	glTexParameterf (rt->GetTextureTarget (), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf (rt->GetTextureTarget (), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf (rt->GetTextureTarget (), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (rt->GetTextureTarget (), GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Set up textures.
	InitializeTextures ();

	return true;
}

bool
BayerRenderer::Initialize (int w,
			   int h,
			   CGcontext context,
			   CGprofile vertexProfile,
			   CGprofile fragmentProfile)
{
	width = w;
	height = h;
	
	return Initialize (context, vertexProfile, fragmentProfile);
}

bool
BayerRenderer::LoadCgPrograms ()
{
	if (!cgIsContext (context)) {
		std::cerr << "ERROR: invalid Cg context" << std::endl;
		return false;
	}

	// Compile and load the vertex program.
	vertexProgram = cgCreateProgramFromFile (
		context,
		CG_SOURCE,
#ifndef OLD
		"bayerv.cg",
#else
		"bayerv_OLD.cg",
#endif
		vertexProfile,
		"bayerv",	// entry point
		NULL);		// arguments
	if (!cgIsProgramCompiled (vertexProgram)) {
		cgCompileProgram (vertexProgram);
	}
	// Enable the appropriate vertex profile and load the vertex program.
	cgGLEnableProfile (vertexProfile);
	cgGLLoadProgram (vertexProgram);

	// Compile and load the fragment program.
	fragmentProgram = cgCreateProgramFromFile (
		context,
		CG_SOURCE,
#ifdef BAYER
		"bayerrawf.cg",
#elif (defined(OLD))
		"bayerf_OLD.cg",
#else
		"bayerf.cg",
#endif
		fragmentProfile,
		"bayerf",	// entry point
		NULL);		// arguments
	if (!cgIsProgramCompiled (fragmentProgram)) {
		cgCompileProgram (fragmentProgram);
	}
	// Enable the appropriate vertex profile and load the vertex program.
	cgGLEnableProfile (fragmentProfile);
	cgGLLoadProgram (fragmentProgram);
	return true;
}

void
BayerRenderer::SetBayer (const GLubyte *bayer) const {
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex_bayer);
	glTexSubImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, bayer);
	Render ();
}

RenderTexture*
BayerRenderer::CreateRenderTexture (int w,
				    int h) const
{
	RenderTexture *rt  = new RenderTexture (RENDERTEXTURE_INIT);
	if (!rt->Initialize(w, h)) {
		std::cerr << "ERROR: RenderTexture initialization failed" << std::endl;
		return NULL;
	}
	rt->BeginCapture ();
	glViewport (0, 0, w, h);
	glMatrixMode (GL_PROJECTION);
  	glLoadIdentity ();
//	gluOrtho2D (0, w, 0, h);
  	glMatrixMode (GL_MODELVIEW);
  	glLoadIdentity ();
	glDisable (GL_LIGHTING);
	glClearColor (0.0, 0.0, 0.0, 1.0);
	rt->EndCapture ();
	return rt;
}

void
BayerRenderer::InitializeTextures ()
{
	glGenTextures (1, &tex_bayer);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex_bayer);
	glTexParameteri (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 1, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
	// Associate this texture handle with the fragment program.
	cgGLSetTextureParameter (cgGetNamedParameter (fragmentProgram, "bayer"), tex_bayer);

	glGenTextures (1, &tex_mask);
	glBindTexture (GL_TEXTURE_2D, tex_mask);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_FLOAT, MASK);
	// Associate this texture handle with the fragment program.
	cgGLSetTextureParameter (cgGetNamedParameter (fragmentProgram, "mask"), tex_mask);
}

void
BayerRenderer::Render () const
{
	// TODO put quad in display list?

	// Render color channels into RenderTexture
	rt->BeginCapture ();
	glClear (GL_COLOR_BUFFER_BIT);

	// Bind the vertex and fragment programs.
	cgGLBindProgram (vertexProgram);
	cgGLEnableProfile (vertexProfile);
	cgGLBindProgram (fragmentProgram);
	cgGLEnableProfile (fragmentProfile);
	
//	cgGLSetParameter1f (cgGetNamedParameter (fragmentProgram, "brightness"), brightness);
//	cgGLSetParameter1f (cgGetNamedParameter (fragmentProgram, "contrast"), contrast);
//	cgGLSetParameter1f (cgGetNamedParameter (fragmentProgram, "grayscale"), grayscale);

	// Enable textures for the fragment shader.
	cgGLEnableTextureParameter (cgGetNamedParameter (fragmentProgram, "bayer"));
	cgGLEnableTextureParameter (cgGetNamedParameter (fragmentProgram, "mask"));

	// Draw a full-screen quad.
	glBegin (GL_QUADS);
	glTexCoord2f (0, 0);          glVertex2f (-1, -1);
	glTexCoord2f (width, 0);      glVertex2f (1, -1);
	glTexCoord2f (width, height); glVertex2f (1, 1);
	glTexCoord2f (0, height);     glVertex2f (-1, 1);
        glEnd();

	// Disable textures for the fragment shader.
	cgGLDisableTextureParameter (cgGetNamedParameter (fragmentProgram, "bayer"));
	cgGLDisableTextureParameter (cgGetNamedParameter (fragmentProgram, "mask"));

	cgGLDisableProfile (vertexProfile);
	cgGLDisableProfile (fragmentProfile);

	rt->EndCapture ();
}
