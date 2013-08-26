/* Copyright (c) 1993 Association of Universities for Research 
 * in Astronomy. All rights reserved. Produced under National   
 * Aeronautics and Space Administration Contract No. NAS5-26555.
 */
/* get_data.c	Read I*2 image data from infile and return as int
 *
 * Format may be raw, net, fits, or hhh.  For FITS input format, header
 * lines get written to outfile.
 *
 * Programmer: R. White		Date: 17 April 1992
 */
#include <stdio.h>
#include <stdlib.h>

extern int  myread();
extern int  test_swap();
extern void swap_bytes();

static void get_raw();
static void get_fits();
static void get_hhh();
extern void fitsread();

extern void
get_data(infile,inname,outfile,a,nx,ny,format)
FILE *infile[2];		/* input file pointers					*/
char *inname[2];		/* input file names						*/
FILE *outfile;			/* output file (only used for FITS)		*/
int  **a;				/* pointer to image array (returned)	*/
int  *nx, *ny;			/* image is NX x NY						*/
char *format;			/* string giving input format			*/
{
	if (strcmp(format,"raw") == 0) {
		get_raw(infile[0], inname[0], a, *nx, *ny, 0);
	} else if (strcmp(format,"net") == 0) {
		get_raw(infile[0], inname[0], a, *nx, *ny, 1);
	} else if (strcmp(format,"fits") == 0) {
		get_fits(infile[0], inname[0], outfile, a, nx, ny);
	} else if (strcmp(format,"hhh") == 0) {
		get_hhh(infile, inname, a, nx, ny);
	} else {
		fprintf(stderr, "get_data: unknown format %s\n", format);
		exit(-1);
	}
	/*
	 * Close files.  Note that two files are used only for format hhh.
	 * infile[0] and infile[1] may point to the same file (usually stdin).
	 */
	fclose(infile[0]);
	if (strcmp(format,"hhh") == 0 && infile[1] != infile[0]) fclose(infile[1]);
}

static void
get_raw(infile,inname,a,nx,ny,swap)
FILE *infile;	/* input file pointer					*/
char *inname;	/* input file name						*/
int  **a;		/* pointer to image array (returned)	*/
int  nx, ny;	/* image is NX x NY						*/
int  swap;		/* non-zero to swap bytes during input	*/
{
int i, j, k;
short *sa;
int tswap;

	*a = (int *) malloc(nx * ny * sizeof(int));
	/*
	 * read a row at a time to minimize page faulting problems
	 */
	sa = (short *) malloc(ny * sizeof(short));
	if (sa == (short *) NULL || *a == (int *) NULL) {
		fprintf(stderr, "insufficient memory\n");
		exit(-1);
	}
	/*
	 * see if byte swapping will be needed
	 */
	if (swap) {
		tswap = test_swap();
	} else {
		tswap = 0;
	}
	/*
	 * read rows
	 */
	for (i=0; i<nx; i++) {
		if(myread(infile, sa, ny*sizeof(short)) != ny*sizeof(short)) {
			fprintf(stderr, "error reading I*2 image (size %d %d) from %s\n",
				nx, ny, inname);
			exit(-1);
		}
		/*
		 * swap if necessary
		 */
		if (tswap) swap_bytes(sa,ny*sizeof(short));
		/*
		 * copy to array A, converting to int
		 */
		k=i*ny;
		for (j=0; j<ny; j++) (*a)[k++] = sa[j];
	}
	free(sa);
}

static void
get_fits(infile,inname,outfile,a,nx,ny)
FILE *infile;	/* input file pointer					*/
char *inname;	/* input file name						*/
FILE *outfile;	/* output file (NULL for none)			*/
int  **a;		/* pointer to image array (returned)	*/
int  *nx, *ny;	/* image is NX x NY						*/
{
	/*
	 * read fits header
	 */
	if (outfile != (FILE *) NULL) {
		/*
		 * 1: pass lines through to outfile
		 * 1: header is multiple of 2880 bytes
		 * 0: lines are not terminated by newline
		 */
		fitsread(infile, inname, outfile, nx, ny, 1, 1, 0);
	} else {
		/*
		 * no output file, don't pass lines through to outfile
		 */
		fitsread(infile, inname, outfile, nx, ny, 0, 1, 0);
	}
	/*
	 * read raw pixel data with byte swapping
	 */
	get_raw(infile, inname, a, *nx, *ny, 1);
}

static void
get_hhh(infile, inname, a,nx,ny)
FILE *infile[2];	/* input file pointers					*/
char *inname[2];	/* input file names						*/
int  **a;			/* pointer to image array (returned)	*/
int  *nx, *ny;		/* image is NX x NY						*/
{
	/*
	 * read fits header
	 * 0: don't pass lines through to output file
	 * 0: header is not multiple of 2880 bytes
	 * 1: lines are terminated by newline
	 */
	fitsread(infile[1], inname[1], (FILE *) NULL, nx, ny, 0, 0, 1);
	/*
	 * read raw pixel data with no byte swapping
	 */
	get_raw(infile[0], inname[0], a, *nx, *ny, 0);
}
