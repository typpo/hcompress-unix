/* Copyright (c) 1993 Association of Universities for Research 
 * in Astronomy. All rights reserved. Produced under National   
 * Aeronautics and Space Administration Contract No. NAS5-26555.
 */
/* dodecode.c	Decode stream of characters on infile and return array
 *
 * This version encodes the different quadrants separately
 *
 * Programmer: R. White		Date: 9 May 1991
 */

#include <stdio.h>
#include <stdlib.h>

#define input_nybble(infile)	input_nbits(infile,4)

extern void qtree_decode();
extern void start_inputing_bits();
extern int input_bit();
extern int input_nbits();

extern void
dodecode(infile,a,nx,ny,nbitplanes)
FILE *infile;
int a[];							/* Array of values to decode			*/
int nx,ny;							/* Array dimensions are [nx][ny]		*/
unsigned char nbitplanes[3];		/* Number of bit planes in quadrants	*/
{
int i, nel, nx2, ny2;

	nel = nx*ny;
	nx2 = (nx+1)/2;
	ny2 = (ny+1)/2;
	/*
	 * initialize a to zero
	 */
	for (i=0; i<nel; i++) a[i] = 0;
	/*
	 * Initialize bit input
	 */
	start_inputing_bits();
	/*
	 * read bit planes for each quadrant
	 */
	qtree_decode(infile, &a[0],          ny, nx2,  ny2,  nbitplanes[0]);
	qtree_decode(infile, &a[ny2],        ny, nx2,  ny/2, nbitplanes[1]);
	qtree_decode(infile, &a[ny*nx2],     ny, nx/2, ny2,  nbitplanes[1]);
	qtree_decode(infile, &a[ny*nx2+ny2], ny, nx/2, ny/2, nbitplanes[2]);
	/*
	 * make sure there is an EOF symbol (nybble=0) at end
	 */
	if (input_nybble(infile) != 0) {
		fprintf(stderr, "dodecode: bad bit plane values\n");
		exit(-1);
	}
	/*
	 * now get the sign bits
	 * Re-initialize bit input
	 */
	start_inputing_bits();
	for (i=0; i<nel; i++) {
		if (a[i] != 0) {
			if (input_bit(infile) != 0) a[i] = -a[i];
		}
	}
}
