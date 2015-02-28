/* rgb_to_bayer.c */

/* Max Smolens
 * max@cs.unc.edu
 * 
 * Converts RGB image to Bayer pattern image
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <magick/api.h>

int
main (int   argc,
      char *argv[])
{
	ExceptionInfo exception;
	Image *image_in,*image_out;
	ImageInfo *image_info;
	const PixelPacket *p;
	char *bayer  = NULL;	/* bayer image start */
	char *bayerp = NULL;	/* current bayer pixel */
	char infile[MaxTextExtent], outfile[MaxTextExtent];
	int x, y, c;
	int verbose = 0;
	int errflag = 0;
	extern int optind, optopt;

	/* parse command-line arguments */
	while ((c = getopt (argc, argv, "v")) != -1) {
		switch (c) {
		case 'v':
			verbose = 1;
			break;
		case '?':
			fprintf (stderr, "Unrecognized option: -%c\n", optopt);
			errflag++;
			break;
		default:
			break;
		}
	}
	
	if (errflag || ((argc-optind) != 2)) {
		fprintf (stderr, "Usage: %s [-v] infile outfile\n", argv[0]);
		fprintf (stderr, "Converts RGB image to Bayer pattern image.\n");
		fprintf(stderr, "\n  -v  print verbose image information\n");
		exit (1);
	}
	    
	strncpy (infile, argv[optind++], MaxTextExtent-1);
	infile[MaxTextExtent-1] = '\0';
	strncpy (outfile, argv[optind++], MaxTextExtent-1);
	outfile[MaxTextExtent-1] = '\0';

	/* initialize the image info structure and read an image */
	InitializeMagick (*argv);
	GetExceptionInfo (&exception);
	image_info = CloneImageInfo ((ImageInfo *) NULL);
	strncpy (image_info->filename, infile, MaxTextExtent);
	image_info->filename[MaxTextExtent-1] = '\0';
	image_in = ReadImage (image_info,&exception);
	if (exception.severity != UndefinedException)
		CatchException (&exception);
	if (image_in == NULL)
		exit (1);
	if (verbose) {
		DescribeImage (image_in, stdout, 0);
	}

	/* allocate bayer image memory */
	bayer = malloc (image_in->columns * image_in->rows * sizeof (char));
	if (bayer == NULL) {
		fprintf (stderr, "ERROR: not enough memory");
		exit (1);
	}
	bayerp = bayer;
	
	/* get image pixels */
	p = AcquireImagePixels (image_in,0,0,image_in->columns,image_in->rows,&exception);
	if (p == NULL)
		MagickError (exception.severity,exception.reason,exception.description);

	/* copy pixels to bayer image */
	for (y = 0; y < image_in->rows; y++) {
		for (x = 0; x < image_in->columns; x++) {
			if (y & 1) { /* y odd -> row starts with GB */
				if (x & 1 ) {

					*bayerp = p->blue;
				} else {
					*bayerp = p->green;
				}
			} else if (x & 1) { /* y even -> row starts with RG */
				*bayerp = p->green;
			} else {
				*bayerp = p->red;
			}
			bayerp++;
			p++;
		}
	}

	/* create bayer image */
	image_out = ConstituteImage (image_in->columns,image_in->rows,"I",CharPixel,bayer,&exception);
	if (image_out == NULL)
		MagickError (exception.severity,exception.reason,exception.description);
	free (bayer);
	
	/* write the bayer image and destroy it */
	SetImageType (image_out, GrayscaleType);
	SetImageDepth (image_out, 8);
	strncpy (image_out->filename, outfile, MaxTextExtent);
	image_out->filename[MaxTextExtent-1] = '\0';
	WriteImage (image_info,image_out);
	if (verbose) {
		DescribeImage (image_out, stdout, 0);
	}
	DestroyImageInfo (image_info);
	DestroyExceptionInfo (&exception);
	DestroyMagick ();
	return (0);
}
