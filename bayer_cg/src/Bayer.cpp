/**
 * @file   bayer.cpp
 * @author Max Smolens
 * @brief  Bayer pattern renderer.
 */

#include <iostream>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <GL/glew.h>
#ifdef WIN32
# include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include "Image.hpp"
#include "BayerRenderer.hpp"
#include "GLUTFPSCounter.hpp"
#include "trackball.h"

// Frames per second counter.
static GLUTFPSCounter fps_counter;

// Cg context.
CGcontext context;

// Handles to vertex profiles and programs.
CGprofile vertexProfile, fragmentProfile;
CGprogram vertexProgram, fragmentProgram;

// Window dimensions.
static int w;
static int h;

// Image dimensions.
static int texw;
static int texh;

// Image options.
static float brightness = 1.0;
static float contrast = 1.0;
static bool grayscale = false;

// Pointer to bayer images.
#define STREAM
#ifdef STREAM
static GLubyte **data = NULL;
#else
static GLubyte *data = NULL;
#endif

static int start_time;
static int end_time;
static int num_frames = 0;

// Bayer renderer.
static BayerRenderer *br;

static void mouse (int button, int state, int x, int y);
static void motion (int x, int y);
static void keyboard (unsigned char key, int x, int y);
static void display ();
static void reshape (int width, int height);
static void idle ();
static void handleCgError ();
static void load_cg_programs ();
static void load_profiles ();
static void read_images (const char *prefix, int num_frames);

int
main (int   argc,
      char *argv[])
{
	const int LEN = 20;
	char prefix[LEN];
	if (argc < 2) {
		std::cerr << "USAGE: " << argv[0] << " <prefix> <num_frames>" << std::endl;
		exit (EXIT_FAILURE);
	}
	snprintf (prefix, LEN, "%s", argv[1]);
	prefix[LEN-1] = '\0';
	// Read image stream or single image.
	if (argc > 2) {
		num_frames = strtol (argv[2], NULL, 10);
		read_images (prefix, num_frames);
	} else {
		num_frames = 1;
		data = new GLubyte*[1];
		if ((data[0] = read_image (argv[1], texw, texh)) == NULL) {
			std::cerr << "ERROR: cannot open file '" << argv[1] << "'." << std::endl;
			exit (EXIT_FAILURE);
		}
	}
	w = texw;
	h = texh;
	
	glutInit (&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize (w, h);
	glutCreateWindow ("Cg Bayer Renderer");

	// Initialize GLEW
	int err = glewInit ();
	if (GLEW_OK != err) {
		std::cerr << "GLEW ERROR: " << glewGetErrorString (err) << std::endl;
		return (EXIT_FAILURE);
	}
	std::cerr << "Cg Bayer Renderer\nUsing GLEW " << glewGetString (GLEW_VERSION) << std::endl;

	glutKeyboardFunc (keyboard);
	glutMouseFunc (mouse);
	glutMotionFunc (motion);
  	glutReshapeFunc (reshape);
  	glutDisplayFunc (display);
  	glutIdleFunc (idle);

	cgSetErrorCallback (handleCgError);
	context = cgCreateContext ();
	load_profiles ();
	load_cg_programs ();
	glClearColor (0.0, 0.0, 0.0, 1.0);

	// Initialize bayer renderer
	br = new BayerRenderer ();
	if (!(br->Initialize (texw, texh, context, vertexProfile, fragmentProfile))) {
		std::cerr << "ERROR: unable to initialize BayerRenderer" << std::endl;
		return (EXIT_FAILURE);
	}
	br->Bind ();
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	tbInit (GLUT_LEFT_BUTTON);
	tbAnimate (true);
	fps_counter.start ();
	start_time = glutGet (GLUT_ELAPSED_TIME);
	glutMainLoop ();
	return (EXIT_SUCCESS);
}

// Glut idle callback.
static void
idle ()
{
	glutPostRedisplay ();
}

// Glut display callback.
static void
display ()
{
	static const GLdouble znear = 1.0;
	static const GLdouble zfar = 10.0;
	static const GLdouble zcenter = -0.5 * (zfar - znear);
	static const GLdouble p = zcenter/znear;
	static int count = 0;
	static int n = 1;
	static int increment = 1;

	bool display_fps = fps_counter.update ();
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glFrustum (-1.0, 1.0, -1.0, 1.0, znear, zfar);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glTranslatef (0.0, 0.0, p); // translate so image initially fills viewport
	tbMatrix ();		// trackball rotation
	glTranslatef (0.0, 0.0, -zcenter); // translate so rotation is around zcenter
	
	// Bind the vertex and fragment programs.
	cgGLBindProgram (vertexProgram);
	cgGLEnableProfile (vertexProfile);
	cgGLBindProgram (fragmentProgram);
	cgGLEnableProfile (fragmentProfile);

	// Bind uniform parameters to the vertex shader.
	cgGLSetStateMatrixParameter(cgGetNamedParameter(vertexProgram, "ModelViewProj"),
				    CG_GL_MODELVIEW_PROJECTION_MATRIX,
				    CG_GL_MATRIX_IDENTITY);

	// Bind uniform parameters to the fragment shader.
	cgGLSetParameter1f (cgGetNamedParameter (fragmentProgram, "brightness"), brightness);
	cgGLSetParameter1f (cgGetNamedParameter (fragmentProgram, "contrast"), contrast);
	cgGLSetParameter1f (cgGetNamedParameter (fragmentProgram, "grayscale"), grayscale);

	// Enable texture for the fragment shader.
	cgGLEnableTextureParameter (cgGetNamedParameter (fragmentProgram, "decal"));

	// Render Bayer data.
	br->SetBayer (data[n-1]);
 	br->Bind ();
	br->EnableTextureTarget ();
	glBegin (GL_QUADS);
 	glTexCoord2i (0, 0);       glVertex3f (p, p, zcenter);
 	glTexCoord2i (texw, 0);    glVertex3f (-p, p, zcenter);
 	glTexCoord2i (texw, texh); glVertex3f (-p, -p, zcenter);
 	glTexCoord2i (0, texh);    glVertex3f (p, -p, zcenter);
        glEnd();
	br->DisableTextureTarget ();

	// Disable texture for the fragment shader.
	cgGLDisableTextureParameter (cgGetNamedParameter (fragmentProgram, "decal"));

	cgGLDisableProfile (vertexProfile);
	cgGLDisableProfile (fragmentProfile);

	// Display frames per second.
	if (display_fps) {
		fprintf (stderr, "FPS: %3.2f\n", fps_counter.get_fps ());
	}

#ifdef SAVEFRAMES
	// Save frame to disk.
	GLubyte *pixelbuffer = new GLubyte[w*h*3];
	glReadPixels (0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixelbuffer);
	char filename[20];
	sprintf (filename, "%s%05d.png", "out/out", count);
	write_image (filename, pixelbuffer, w, h);
	delete [] pixelbuffer;
	std::cerr<< "readpixels" << std::endl;
#endif
	glutSwapBuffers ();

	// Increment frame if enough time has elapsed.
	static const float NTSC_FRAMERATE = 1.0/29.97;
	end_time = glutGet (GLUT_ELAPSED_TIME);
	float elapsed = (end_time - start_time) / 1e3;
	if (elapsed > NTSC_FRAMERATE) {
		count++;
		n += increment;
		if (n < 1) {
			n = 1;
			increment = 1;
		} else if (n > (num_frames - 1)) {
			n = num_frames;
			increment = -1;
#ifdef SAVEFRAMES
//			exit (0);
#endif
		}
		start_time = glutGet (GLUT_ELAPSED_TIME);
	}
}

// Glut reshape callback.
static void
reshape (int width,
	 int height)
{
	w = width;
	h = height;
	tbReshape (w, h);
	glViewport (0, 0, w, h);
}

// Glut mouse callback.
static void
mouse (int button,
       int state,
       int x,
       int y)
{
	tbMouse (button, state, x, y);
}

// Glut mouse motion callback.
static void
motion (int x,
	int y)
{
	tbMotion (x, y);
//	glutPostRedisplay ();
}

// Glut keyboard callback.
static void
keyboard (unsigned char key,
	  int x,
	  int y)
{
	static const float MAXVAL = 10.0;
	switch (key) {
	case 27:
	case 'Q':
	case 'q':
		cgDestroyContext (context);
		exit (EXIT_SUCCESS);
		break;
	case 'G':
	case 'g':
		grayscale = !grayscale;
		break;
	case '<':
		brightness -= 0.1;
		if (brightness < 0.0) {
			brightness = 0.0;
		}
		break;
	case '>':
		brightness += 0.1;
		if (brightness > MAXVAL) {
			brightness = MAXVAL;
		}
		break;
	case '[':
		contrast -= 0.1;
		if (contrast < 0.0) {
			contrast = 0.0;
		}
		break;
	case ']':
		contrast += 0.1;
		if (contrast > MAXVAL) {
			contrast = MAXVAL;
		}
		break;
	case 'F':
	case 'f':
		glutFullScreen ();
		break;
	default:
		break;
	}
	glutPostRedisplay ();
}

// Cg error callback.
static void
handleCgError ()
{
	CGerror error = cgGetError ();
	if (error) {
		std::cerr << cgGetErrorString (error) << std::endl;
		std::cerr <<  cgGetLastListing (context) << std::endl;
		exit (EXIT_FAILURE);
	}
}

// Loads the vertex and fragment profiles.
static void
load_profiles ()
{
	if ((vertexProfile = cgGLGetLatestProfile (CG_GL_VERTEX)) == CG_PROFILE_UNKNOWN) {
		std::cerr << "ERROR: cannot load vertex profile." << std::endl;
		exit (EXIT_FAILURE);
	}
	cgGLSetOptimalOptions (vertexProfile);

	if ((fragmentProfile = cgGLGetLatestProfile (CG_GL_FRAGMENT)) == CG_PROFILE_UNKNOWN) {
		std::cerr << "ERROR: cannot load fragment profile." << std::endl;
		exit (EXIT_FAILURE);
	}
	cgGLSetOptimalOptions (fragmentProfile);
}

// Compiles and loads vertex and fragment programs.
static void
load_cg_programs ()
{
	if (!cgIsContext (context)) {
		std::cerr <<  "ERROR: invalid Cg context" << std::endl;
		exit (EXIT_FAILURE);
	}

	// Compile and load the vertex program.
	vertexProgram = cgCreateProgramFromFile (
		context,
		CG_SOURCE,
		"imagev.cg",
		vertexProfile,
		"imagev",	// entry point
		NULL);		// arguments
	if (!cgIsProgramCompiled (vertexProgram)) {
		cgCompileProgram (vertexProgram);
	}
	// Enable the appropriate vertex profile and load the vertex program.
	cgGLLoadProgram (vertexProgram);

	// Compile and load the fragment program.
	fragmentProgram = cgCreateProgramFromFile (
		context,
		CG_SOURCE,
		"imagef.cg",
		fragmentProfile,
		"imagef",	// entry point
		NULL);		// arguments
	if (!cgIsProgramCompiled (fragmentProgram)) {
		cgCompileProgram (fragmentProgram);
	}
	// Enable the appropriate vertex profile and load the vertex program.
	cgGLLoadProgram (fragmentProgram);
}

// Reads a stream of images from the disk.
static void
read_images (const char *prefix,
	     int num_frames)
{
	char filename[40];
	int prefix_len = strlen (prefix);
	sprintf (filename, "%s", prefix);
	data = new GLubyte*[num_frames];
	for (int i = 1; i <= num_frames; i++) {
		sprintf (filename+prefix_len, "%08d.png.png", i);
		if ((data[i-1] = read_image (filename, texw, texh)) == NULL) {
			std::cerr << "ERROR: cannot open file '" << filename << "'." << std::endl;
		}
		std::cerr << filename << std::endl;
	}
}
