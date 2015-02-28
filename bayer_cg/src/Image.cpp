#include "Image.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <magick/api.h>

GLubyte *
read_image (const char *filename,
	    int &width,
	    int &height)
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

	// Initialize the image info structure and read image.
	GetExceptionInfo (&exception);
	image_info = CloneImageInfo ((ImageInfo *) NULL);
	strncpy (image_info->filename, filename, MaxTextExtent);
	image_info->filename[MaxTextExtent-1] = '\0';
	image = ReadImage (image_info, &exception);
	if (image == NULL) {
		MagickError(exception.severity, exception.reason, exception.description);
		return (NULL);
	}
	width = w = image->columns;
	height = h = image->rows;

	// Flip image.
	flip_image = FlipImage (image, &exception);
	DestroyImage (image);
	if (!flip_image) {
		MagickError (exception.severity, exception.reason, exception.description);
		return (NULL);
	}
	image = flip_image;
	
	// Copy pixels to array.
	pixels = AcquireImagePixels (image, 0, 0, image->columns, image->rows, &exception);
	if (!pixels) {
		MagickError(exception.severity, exception.reason, exception.description);
		return (NULL);
	}
	int type = 0;
	if (type == 0) {
		data = new GLubyte[w * h];
		for (j = 0; j < image->rows; j++) {
			for (i = 0; i < image->columns; i++) {
				data[j*w+i] = pixels[i + image->columns*j].red;
			}
		}
	} else {
		data = new GLubyte[w * h * 3];
		for (j = 0; j < image->rows; j++) {
			for (i = 0; i < image->columns; i++) {
				int idx = (j * w +i ) * 3;
				data[idx+0] = pixels[i + image->columns*j].red;
				data[idx+1] = pixels[i + image->columns*j].green;
				data[idx+2] = pixels[i + image->columns*j].blue;
			}
		}
	}
	DestroyImageInfo (image_info);
	DestroyExceptionInfo (&exception);
	DestroyImage (image);
	return data;
}

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
