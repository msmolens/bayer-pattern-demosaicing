#include <iostream>
#include "RenderTexture.h"
#include "BayerRenderer.hpp"

const int BayerRenderer::REQUIRED_TEXTURE_UNITS = 4;
const float BayerRenderer::OFFSET = 0.375;
const float BayerRenderer::BLEND[4] = {0.0, 0.0, 0.0, 0.5};
const char *BayerRenderer::RENDERTEXTURE_INIT = "rgba texRECT";

const int BayerRenderer::OFFSET_RED[2]    = {-1,  0};
const int BayerRenderer::OFFSET_BLUE[2]   = { 0, -1};
const int BayerRenderer::OFFSET_GREEN1[2] = { 0,  0};
const int BayerRenderer::OFFSET_GREEN2[2] = {-1, -1};

BayerRenderer::BayerRenderer () : width (0),
				  height (0)
{
}

BayerRenderer::~BayerRenderer ()
{
	if (rt != NULL) {
		delete rt;
	}
	glDeleteTextures (5, tex);
	glDeleteLists (texMagList, 6);
}

bool
BayerRenderer::Initialize ()
{
	int num_tex_units;

	// Check for required extensions
	if (!GLEW_NV_texture_rectangle) {
		std::cerr << "ERROR: GL_NV_texture_rectangle not supported" << std::endl;
		return false;
	}
	if (!GLEW_ARB_texture_env_combine) {
		std::cerr << "ERROR: GL_ARB_texture_env_combine not supported" << std::endl;
		return false;
	}
	if (!GLEW_ARB_multitexture) {
		std::cerr << "ERROR: GL_ARB_multitexture not supported" << std::endl;
		return false;
	}

	// Check for required number of texture units
	glGetIntegerv (GL_MAX_TEXTURE_UNITS, &num_tex_units);
	if (num_tex_units < REQUIRED_TEXTURE_UNITS) {
		std::cerr << "ERROR: Requires " << REQUIRED_TEXTURE_UNITS << " texture units (" << num_tex_units << " available)" << std::endl;
		return false;
	}

	// Create RenderTexture
	rt = CreateRenderTexture (width, height);

	// Set up bilinear interpolation
	rt->Bind ();
	glTexParameterf (rt->GetTextureTarget (), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf (rt->GetTextureTarget (), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf (rt->GetTextureTarget (), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (rt->GetTextureTarget (), GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Set up textures
	InitializeTextures ();

	// Create display lists
	CreateMagnificationDisplayList ();
	CreateMinificationDisplayLists ();
	
	return true;
}

bool
BayerRenderer::Initialize (int w,
			   int h)
{
	width = w;
	height = h;
	
	return Initialize ();
}

void
BayerRenderer::SetBayer (const GLubyte *bayer) const {
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[BAYER]);
	glTexSubImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, bayer);
	Render ();
}

void
BayerRenderer::Bind () const
{
	rt->Bind ();
}

void
BayerRenderer::EnableTextureTarget () const
{
	rt->EnableTextureTarget ();
}

void
BayerRenderer::DisableTextureTarget () const
{
	rt->DisableTextureTarget ();
}

RenderTexture*
BayerRenderer::CreateRenderTexture (int w,
				    int h) const
{
	RenderTexture *rt  = new RenderTexture(RENDERTEXTURE_INIT);
	if (!rt->Initialize(w, h)) {
		std::cerr << "ERROR: RenderTexture initialization failed" << std::endl;
		return NULL;
	}
	rt->BeginCapture ();
	glViewport (0, 0, w, h);
	glMatrixMode (GL_PROJECTION);
  	glLoadIdentity ();
	gluOrtho2D (0, w, 0, h);
  	glMatrixMode (GL_MODELVIEW);
  	glLoadIdentity();
	glDisable(GL_LIGHTING);
	glClearColor (0.0, 0.0, 0.0, 1.0);
	rt->EndCapture ();
	return rt;
}

void
BayerRenderer::InitializeTextures ()
{
	glGenTextures (5, tex);

	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[BAYER]);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 1, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[RED]);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 4, width/2, height/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
	
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[BLUE]);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 4, width/2, height/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
	
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[GREEN1]);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 4, width/2, height/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
	
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[GREEN2]);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 4, width/2, height/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
}

void
BayerRenderer::Render () const
{
	// Render color channels into RenderTexture
	rt->BeginCapture ();
	glClear (GL_COLOR_BUFFER_BIT);
	glEnable (GL_TEXTURE_RECTANGLE_NV);
	glMatrixMode (GL_MODELVIEW);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[BAYER]);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glCallList (texMinListRed);
	glCallList (texMinListBlue);
	glCallList (texMinListGreen1);
	glCallList (texMinListGreen2);
	glLoadIdentity ();
	glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// Copy from pbuffer into textures
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[RED]);
	glCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, width/2, height/2);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[BLUE]);
	glCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, height/2, width/2, height/2);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[GREEN1]);
	glCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, width/2, height/2, width/2, height/2);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[GREEN2]);
	glCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, width/2, 0, width/2, height/2);

	// Render magnified and interpolated color channels
	//glClear (GL_COLOR_BUFFER_BIT); // we overwrite same area, clear should not be necessary

	// Set up green1
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[GREEN1]);
	glEnable (GL_TEXTURE_RECTANGLE_NV);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// Set up green2
	// Blend with green1
	glActiveTexture (GL_TEXTURE1);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[GREEN2]);
	glEnable (GL_TEXTURE_RECTANGLE_NV);
	glTexEnvfv (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, BLEND);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
	glTexEnvf (GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvf (GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf (GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE1);
	glTexEnvf (GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvf (GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
	glTexEnvf (GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
//      glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
//	glTexEnvf (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
//	glTexEnvf (GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

	// Set up red
	glActiveTexture (GL_TEXTURE2);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[RED]);
	glEnable (GL_TEXTURE_RECTANGLE_NV);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);

	// Set up blue
	glActiveTexture (GL_TEXTURE3);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, tex[BLUE]);
	glEnable (GL_TEXTURE_RECTANGLE_NV);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);

	// Render all channels
	glCallList (texMagList);

	// Clean up
	glActiveTexture (GL_TEXTURE3);
	glDisable (GL_TEXTURE_RECTANGLE_NV);
	glActiveTexture (GL_TEXTURE2);
	glDisable (GL_TEXTURE_RECTANGLE_NV);
	glActiveTexture (GL_TEXTURE1);
	glDisable (GL_TEXTURE_RECTANGLE_NV);
	glActiveTexture (GL_TEXTURE0);
	glDisable (GL_TEXTURE_RECTANGLE_NV);

	rt->EndCapture ();
}

void
BayerRenderer::CreateMagnificationDisplayList ()
{
	texMagList = glGenLists (1);
	glNewList (texMagList, GL_COMPILE);
	glBegin (GL_QUADS);

	glMultiTexCoord2i (GL_TEXTURE0, 0, 0);
	glMultiTexCoord2i (GL_TEXTURE1, 0, 0);
	glMultiTexCoord2i (GL_TEXTURE2, 0, 0);
	glMultiTexCoord2i (GL_TEXTURE3, 0, 0);
	glVertex2i (0, 0);

	glMultiTexCoord2i (GL_TEXTURE0, width/2, 0);
	glMultiTexCoord2i (GL_TEXTURE1, width/2, 0);
	glMultiTexCoord2i (GL_TEXTURE2, width/2, 0);
	glMultiTexCoord2i (GL_TEXTURE3, width/2, 0);
	glVertex2i (width, 0);
	
	glMultiTexCoord2i (GL_TEXTURE0, width/2, height/2);
	glMultiTexCoord2i (GL_TEXTURE1, width/2, height/2);
	glMultiTexCoord2i (GL_TEXTURE2, width/2, height/2);
	glMultiTexCoord2i (GL_TEXTURE3, width/2, height/2);
	glVertex2i (width, height);

	glMultiTexCoord2i (GL_TEXTURE0, 0, height/2);
	glMultiTexCoord2i (GL_TEXTURE1, 0, height/2);
	glMultiTexCoord2i (GL_TEXTURE2, 0, height/2);
	glMultiTexCoord2i (GL_TEXTURE3, 0, height/2);
	glVertex2i (0, height);

	glEnd();
	glEndList ();
}

void
BayerRenderer::CreateMinificationDisplayLists ()
{
	texMinListMain   = glGenLists (5);
	texMinListRed    = texMinListMain + 1;
	texMinListBlue   = texMinListMain + 2;
	texMinListGreen1 = texMinListMain + 3;
	texMinListGreen2 = texMinListMain + 4;

	// Draw quarter-size textured quad
	glNewList (texMinListMain, GL_COMPILE);
	glBegin (GL_QUADS);
	glTexCoord2i (0, 0);          glVertex2i (0, 0);
	glTexCoord2i (width, 0);      glVertex2i (width/2, 0);
	glTexCoord2i (width, height); glVertex2i (width/2, height/2);
	glTexCoord2i (0, height);     glVertex2i (0, height/2);
	glEnd();
	glLoadIdentity ();
	glMatrixMode (GL_MODELVIEW);
	glEndList ();

	// Render color channels
	//  --------
	// | B | G1 |
	// |---|----| 
	// | R | G2 |
	//  --------
	// Note: need colormasks for multitexturing
	// Red
	glNewList (texMinListRed, GL_COMPILE);
	glColorMask (GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
	glLoadIdentity ();
	glMatrixMode (GL_TEXTURE);
	glLoadIdentity ();
	glTranslatef (OFFSET + OFFSET_RED[0], OFFSET + OFFSET_RED[1], 0.0);
	glCallList (texMinListMain);
	glEndList ();

	// Blue
	glNewList (texMinListBlue, GL_COMPILE);
	glColorMask (GL_FALSE, GL_FALSE, GL_TRUE, GL_TRUE);
	glTranslatef (0.0, height/2, 0.0);
	glMatrixMode (GL_TEXTURE);
	glLoadIdentity ();
	glTranslatef (OFFSET + OFFSET_BLUE[0], OFFSET + OFFSET_BLUE[1], 0.0);
	glCallList (texMinListMain);
	glEndList ();

	// Green1
	glNewList (texMinListGreen1, GL_COMPILE);
	glColorMask (GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);
	glLoadIdentity ();
	glTranslatef (width/2, height/2, 0.0);
	glMatrixMode (GL_TEXTURE);
	glLoadIdentity ();
	glTranslatef (OFFSET + OFFSET_GREEN1[0], OFFSET + OFFSET_GREEN1[1], 0.0);
	glCallList (texMinListMain);
	glEndList ();

	// Green2
	glNewList (texMinListGreen2, GL_COMPILE);
	glLoadIdentity ();
	glTranslatef (width/2, 0.0, 0.0);
	glMatrixMode (GL_TEXTURE);
	glLoadIdentity ();
	glTranslatef (OFFSET + OFFSET_GREEN2[0], OFFSET + OFFSET_GREEN2[1], 0.0);
	glCallList (texMinListMain);
	glEndList ();
}
