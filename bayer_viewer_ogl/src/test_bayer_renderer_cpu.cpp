/**
 * @file   test_bayer_renderer_cpu.cpp
 * @author Max Smolens
 * @brief  Demonstration of Bayer pattern renderer using CPU.
 */

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <magick/api.h>
#include "GLUTFPSCounter.hpp"
#include "BayerRendererCPU.hpp"
#include <GL/glut.h>

// types for loading images
enum
{
	GRAYSCALE,
	RGB
};

// ids for menu
enum
{
	MENU_SCREENSHOT,
	MENU_QUIT
};

#define SCREENSHOT_FILENAME "out.tiff"

static int width = 640;
static int height = 480;

// ptr to bayer image
static GLubyte *bayer = NULL;

static GLUTFPSCounter fps_counter;
static BayerRendererCPU *br;

/// reads image file into array
GLubyte *
read_image (const char *filename,
	    int         type,
	    int        *width,
	    int        *height)
{
	ExceptionInfo exception;
	Image *image;
	Image *flip_image;
	ImageInfo *image_info;
	const PixelPacket *pixels;
	GLubyte *data = NULL;
	unsigned int i;
	unsigned int j;
	int w, h;

	// initialize the image info structure and read image
	GetExceptionInfo (&exception);
	image_info = CloneImageInfo ((ImageInfo *) NULL);
	strncpy (image_info->filename, filename, MaxTextExtent);
	image_info->filename[MaxTextExtent-1] = '\0';
	image = ReadImage (image_info, &exception);
	if (image == NULL) {
		MagickError(exception.severity, exception.reason, exception.description);
		return (NULL);
	}
	*width = w = image->columns;
	*height = h = image->rows;

	// flip image
	flip_image = FlipImage (image, &exception);
	DestroyImage (image);
	if (!flip_image) {
		MagickError (exception.severity, exception.reason, exception.description);
		return (NULL);
	}
	image = flip_image;
	
	// copy pixels to array
	pixels = AcquireImagePixels (image, 0, 0, image->columns, image->rows, &exception);
	if (!pixels) {
		MagickError(exception.severity, exception.reason, exception.description);
		return (NULL);
	}
	if (type == GRAYSCALE) {
		data = new GLubyte[w * h];
		for (j = 0; j < image->rows; j++) {
			for (i = 0; i < image->columns; i++) {
				data[j*w+i] = pixels[i + image->columns*j].red;
			}
		}
	} else {
		data = new GLubyte[w * h * 4];
		for (j = 0; j < image->rows; j++) {
			for (i = 0; i < image->columns; i++) {
				int idx = (j * w +i ) * 4;
				data[idx+0] = pixels[i + image->columns*j].red;
				data[idx+1] = pixels[i + image->columns*j].green;
				data[idx+2] = pixels[i + image->columns*j].blue;
				data[idx+3] = pixels[i + image->columns*j].opacity;
			}
		}
	}
	DestroyImageInfo (image_info);
	DestroyExceptionInfo (&exception);
	DestroyImage (image);
	return data;
}

/// writes data to RGB image file
int
write_image (const char *filename,
	     const unsigned char *data,
	     int width,
	     int height)

{
	ExceptionInfo exception;
	Image *image;
	Image *flip_image;
	ImageInfo *image_info;

	// initialize the image
	GetExceptionInfo(&exception);
	image = ConstituteImage (width, height, "RGB", CharPixel, data, &exception);
	if (image == NULL) {
		MagickError (exception.severity,exception.reason,exception.description);
		return (1);
	}

	// image is flipped vertically, with data coming from frame buffer
	flip_image = FlipImage (image, &exception);
	DestroyImage (image);
	if (!flip_image) {
		MagickError (exception.severity, exception.reason, exception.description);
		return (1);
	}
	image = flip_image;
	image_info = CloneImageInfo ((ImageInfo *) NULL);
	image_info->compression = NoCompression;
	SetImageDepth (image, 8);
	strncpy (image->filename, filename, MaxTextExtent);
	image->filename[MaxTextExtent-1] = '\0';
	if (!WriteImage (image_info, image)) {
		MagickError (exception.severity,exception.reason,exception.description);
		return (1);
	}
	DestroyImageInfo (image_info);
	DestroyImage (image);
	return (0);
}

/// glut idle callback
void
idle ()
{
	glutPostRedisplay ();
}

/// glut display callback
void
display ()
{
	bool display_fps = fps_counter.update ();
	
	glClear (GL_COLOR_BUFFER_BIT);
	br->SetBayer (bayer);	// bayer is ptr to bayer image
	br->Bind ();
	br->EnableTextureTarget ();
	glBegin (GL_QUADS);
	glTexCoord2i (0, 0);          glVertex2i (0, 0);
	glTexCoord2i (width, 0);      glVertex2i (width, 0);
	glTexCoord2i (width, height); glVertex2i (width, height);
	glTexCoord2i (0, height);     glVertex2i (0, height);
	glEnd();
	br->DisableTextureTarget ();
  	glutSwapBuffers ();

	// print FPS
	if (display_fps) {
		printf ("FPS: %3.2f\n", fps_counter.get_fps ());
	}
}

/// glut reshape callback
void
reshape (int w,
	 int h)
{
  	glViewport (0, 0, w, h);

	glMatrixMode (GL_PROJECTION);
  	glLoadIdentity ();
	gluOrtho2D (0, w, 0, h);

  	glMatrixMode (GL_MODELVIEW);
  	glLoadIdentity();

  	glutPostRedisplay ();
}

/// takes screenshot of rectangle with origin (x,y)
void
screenshot (int x,
            int y)
{
	unsigned char *p = NULL;
	glReadBuffer (GL_FRONT);
	p = new unsigned char[width * height * 3];
	glReadPixels (x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, p);
	if (write_image (SCREENSHOT_FILENAME, p, width, height) != 0) {
		std::cerr << "ERROR: unable to write file '" << SCREENSHOT_FILENAME << "'" << std::endl;
	}
	delete [] p;
	glReadBuffer (GL_BACK);
}

/// menu handler
void
select_from_menu (int command)
{
	switch (command) {
	case MENU_SCREENSHOT:
		screenshot (0, 0);
		break;
	case MENU_QUIT:
		if (br != NULL) {
			delete br;
		}
		exit (EXIT_SUCCESS);
		break;
	}
}

/// glut keyboard callback
void
on_key_press (unsigned char key,
	      int x,
	      int y)
{
	switch (key)
	{
	case 'Q':
	case 'q':
	case 27:
		select_from_menu (MENU_QUIT);
		break;
	case 'S':
	case 's':
		select_from_menu (MENU_SCREENSHOT);
		break;
  	};
  	glutPostRedisplay();
}

/// builds glut menu
int
build_glut_menu ()
{
	int menu;

	menu = glutCreateMenu (select_from_menu);
	glutAddMenuEntry ("Screenshot (s)", MENU_SCREENSHOT);
	glutAddMenuEntry ("Quit", MENU_QUIT);
	glutAttachMenu (GLUT_RIGHT_BUTTON);
	return menu;
}

/// main function
int
main (int   argc,
      char *argv[])
{
	char bayer_filename[MaxTextExtent];
	int w, h;

	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <bayer image filename>" << std::endl;
		std::cerr << "Displays CPU-demosaiced Bayer pattern image." << std::endl;
		return (EXIT_FAILURE);
	}

	// copy filenames from argv
	strncpy (bayer_filename, argv[1], MaxTextExtent-1);
	bayer_filename[MaxTextExtent-1] = '\0';
	
	// read bayer image
	if ((bayer = read_image (bayer_filename, GRAYSCALE, &w, &h)) == NULL) {
		std::cerr << "ERROR: unable to read image '" << bayer_filename << "'" << std::endl;
 		return (EXIT_FAILURE);
	}

	// init glut
 	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA);	
	glutInitWindowSize (width, height);
  	glutInitWindowPosition (0, 0);
  	glutCreateWindow ("Bayer Viewer (CPU)");

	// Initialize GLEW
	int err = glewInit ();
	if (GLEW_OK != err) {
		std::cerr << "GLEW ERROR: " << glewGetErrorString (err) << std::endl;
		return (EXIT_FAILURE);
	}
	
	// init glut callbacks
	glutKeyboardFunc (on_key_press);
  	glutReshapeFunc (reshape);
  	glutDisplayFunc (display);
  	glutIdleFunc (idle);
	build_glut_menu ();

  	glPixelStorei (GL_PACK_ALIGNMENT, 1);
  	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	glClearColor (0.0, 0.0, 0.0, 1.0);

	// init bayer renderer
	br = new BayerRendererCPU ();
	if (!(br->Initialize (width, height))) {
		std::cerr << "ERROR: unable to initialize BayerRenderer" << std::endl;
		return (EXIT_FAILURE);
	}
	br->Bind ();
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	fps_counter.start ();
	glutMainLoop ();
	return (EXIT_SUCCESS);
}
