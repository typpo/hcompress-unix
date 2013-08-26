/* Copyright (c) 1993 Association of Universities for Research 
 * in Astronomy. All rights reserved. Produced under National   
 * Aeronautics and Space Administration Contract No. NAS5-26555.
 */
/* doencode.c	Encode 2-D array and write stream of characters on outfile
 *
 * This version assumes that A is positive.
 *
 * Programmer: R. White		Date: 7 May 1991
 */

#include <stdio.h>

#define output_nybble(outfile,c)	output_nbits(outfile,c,4)

extern void qtree_encode();
extern void start_outputing_bits();
extern void done_outputing_bits();
extern void output_nbits();

extern void
doencode(outfile,a,nx,ny,nbitplanes)
FILE *outfile;						/* output data stream					*/
int a[];							/* Array of values to encode			*/
int nx,ny;							/* Array dimensions [nx][ny]			*/
unsigned char nbitplanes[3];		/* Number of bit planes in quadrants	*/
{
int nx2, ny2;

	nx2 = (nx+1)/2;
	ny2 = (ny+1)/2;
	/*
	 * Initialize bit output
	 */
	start_outputing_bits();
	/*
	 * write out the bit planes for each quadrant
	 */
	qtree_encode(outfile, &a[0],          ny, nx2,  ny2,  nbitplanes[0]);
	qtree_encode(outfile, &a[ny2],        ny, nx2,  ny/2, nbitplanes[1]);
	qtree_encode(outfile, &a[ny*nx2],     ny, nx/2, ny2,  nbitplanes[1]);
	qtree_encode(outfile, &a[ny*nx2+ny2], ny, nx/2, ny/2, nbitplanes[2]);
	/*
	 * Add zero as an EOF symbol
	 */
	output_nybble(outfile, 0);
	done_outputing_bits(outfile);
}
