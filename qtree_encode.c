/* Copyright (c) 1993 Association of Universities for Research 
 * in Astronomy. All rights reserved. Produced under National   
 * Aeronautics and Space Administration Contract No. NAS5-26555.
 */
/* qtree_encode.c	Encode values in quadrant of 2-D array using binary
 *					quadtree coding for each bit plane.  Assumes array is
 *					positive.
 *
 * Programmer: R. White		Date: 15 May 1991
 */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/*
 * Huffman code values and number of bits in each code
 */
static int code[16] =
	{
	0x3e, 0x00, 0x01, 0x08, 0x02, 0x09, 0x1a, 0x1b,
	0x03, 0x1c, 0x0a, 0x1d, 0x0b, 0x1e, 0x3f, 0x0c
	};
static int ncode[16] =
	{
	6,    3,    3,    4,    3,    4,    5,    5,
	3,    5,    4,    5,    4,    5,    6,    4
	};

/*
 * variables for bit output to buffer when Huffman coding
 */
static int bitbuffer, bits_to_go;

/*
 * macros to write out 4-bit nybble, Huffman code for this value
 */
#define output_nybble(outfile,c)	output_nbits(outfile,c,4)
#define output_huffman(outfile,c)	output_nbits(outfile,code[c],ncode[c])

extern void output_nbits();

static void qtree_onebit();
static void qtree_reduce();
static int  bufcopy();
static void write_bdirect();

extern void
qtree_encode(outfile,a,n,nqx,nqy,nbitplanes)
FILE *outfile;
int a[];
int n;								/* physical dimension of row in a		*/
int nqx;							/* length of row						*/
int nqy;							/* length of column (<=n)				*/
int nbitplanes;						/* number of bit planes to output		*/
{
int log2n, i, k, bit, b, bmax, nqmax, nqx2, nqy2, nx, ny;
unsigned char *scratch, *buffer;

	/*
	 * log2n is log2 of max(nqx,nqy) rounded up to next power of 2
	 */
	nqmax = (nqx>nqy) ? nqx : nqy;
	log2n = log((float) nqmax)/log(2.0)+0.5;
	if (nqmax > (1<<log2n)) {
		log2n += 1;
	}
	/*
	 * initialize buffer point, max buffer size
	 */
	nqx2 = (nqx+1)/2;
	nqy2 = (nqy+1)/2;
	bmax = (nqx2*nqy2+1)/2;
	/*
	 * We're indexing A as a 2-D array with dimensions (nqx,nqy).
	 * Scratch is 2-D with dimensions (nqx/2,nqy/2) rounded up.
	 * Buffer is used to store string of codes for output.
	 */
	scratch = (unsigned char *) malloc(2*bmax);
	buffer = (unsigned char *) malloc(bmax);
	if ((scratch == (unsigned char *) NULL) ||
		(buffer  == (unsigned char *) NULL)) {
		fprintf(stderr, "qtree_encode: insufficient memory\n");
		exit(-1);
	}
	/*
	 * now encode each bit plane, starting with the top
	 */
	for (bit=nbitplanes-1; bit >= 0; bit--) {
		/*
		 * initial bit buffer
		 */
		b = 0;
		bitbuffer = 0;
		bits_to_go = 0;
		/*
		 * on first pass copy A to scratch array
		 */
		qtree_onebit(a,n,nqx,nqy,scratch,bit);
		nx = (nqx+1)>>1;
		ny = (nqy+1)>>1;
		/*
		 * copy non-zero values to output buffer, which will be written
		 * in reverse order
		 */
		if (bufcopy(scratch,nx*ny,buffer,&b,bmax)) {
			/*
			 * quadtree is expanding data,
			 * change warning code and just fill buffer with bit-map
			 */
			write_bdirect(outfile,a,n,nqx,nqy,scratch,bit);
			goto bitplane_done;
		}
		/*
		 * do log2n reductions
		 */
		for (k = 1; k<log2n; k++) {
			qtree_reduce(scratch,ny,nx,ny,scratch);
			nx = (nx+1)>>1;
			ny = (ny+1)>>1;
			if (bufcopy(scratch,nx*ny,buffer,&b,bmax)) {
				write_bdirect(outfile,a,n,nqx,nqy,scratch,bit);
				goto bitplane_done;
			}
		}
		/*
		 * OK, we've got the code in buffer
		 * Write quadtree warning code, then write buffer in reverse order
		 */
		output_nybble(outfile,0xF);
		if (b==0) {
			if (bits_to_go>0) {
				/*
				 * put out the last few bits
				 */
				output_nbits(outfile, bitbuffer & ((1<<bits_to_go)-1),
					bits_to_go);
			} else {
				/*
				 * have to write a zero nybble if there are no 1's in array
				 */
				output_huffman(outfile,0);
			}
		} else {
			if (bits_to_go>0) {
				/*
				 * put out the last few bits
				 */
				output_nbits(outfile, bitbuffer & ((1<<bits_to_go)-1),
					bits_to_go);
			}
			for (i=b-1; i>=0; i--) {
				output_nbits(outfile,buffer[i],8);
			}
		}
		bitplane_done: ;
	}
	free(buffer);
	free(scratch);
}

/*
 * copy non-zero codes from array to buffer
 */
static int
bufcopy(a,n,buffer,b,bmax)
unsigned char a[];
int n;
unsigned char buffer[];
int *b;
int bmax;
{
int i;

	for (i = 0; i < n; i++) {
		if (a[i] != 0) {
			/*
			 * add Huffman code for a[i] to buffer
			 */
			bitbuffer |= code[a[i]] << bits_to_go;
			bits_to_go += ncode[a[i]];
			if (bits_to_go >= 8) {
				buffer[*b] = bitbuffer & 0xFF;
				*b += 1;
				/*
				 * return warning code if we fill buffer
				 */
				if (*b >= bmax) return(1);
				bitbuffer >>= 8;
				bits_to_go -= 8;
			}
		}
	}
	return(0);
}

/*
 * Do first quadtree reduction step on bit BIT of array A.
 * Results put into B.
 * 
 */
static void
qtree_onebit(a,n,nx,ny,b,bit)
int a[];
int n;
int nx;
int ny;
unsigned char b[];
int bit;
{
int i, j, k;
int b0, b1, b2, b3;
int s10, s00;

	/*
	 * use selected bit to get amount to shift
	 */
	b0 = 1<<bit;
	b1 = b0<<1;
	b2 = b0<<2;
	b3 = b0<<3;
	k = 0;							/* k is index of b[i/2,j/2]	*/
	for (i = 0; i<nx-1; i += 2) {
		s00 = n*i;					/* s00 is index of a[i,j]	*/
		s10 = s00+n;				/* s10 is index of a[i+1,j]	*/
		for (j = 0; j<ny-1; j += 2) {
			b[k] = ( ( a[s10+1]     & b0)
				   | ((a[s10  ]<<1) & b1)
				   | ((a[s00+1]<<2) & b2)
				   | ((a[s00  ]<<3) & b3) ) >> bit;
			k += 1;
			s00 += 2;
			s10 += 2;
		}
		if (j < ny) {
			/*
			 * row size is odd, do last element in row
			 * s00+1,s10+1 are off edge
			 */
			b[k] = ( ((a[s10  ]<<1) & b1)
				   | ((a[s00  ]<<3) & b3) ) >> bit;
			k += 1;
		}
	}
	if (i < nx) {
		/*
		 * column size is odd, do last row
		 * s10,s10+1 are off edge
		 */
		s00 = n*i;
		for (j = 0; j<ny-1; j += 2) {
			b[k] = ( ((a[s00+1]<<2) & b2)
				   | ((a[s00  ]<<3) & b3) ) >> bit;
			k += 1;
			s00 += 2;
		}
		if (j < ny) {
			/*
			 * both row and column size are odd, do corner element
			 * s00+1, s10, s10+1 are off edge
			 */
			b[k] = ( ((a[s00  ]<<3) & b3) ) >> bit;
			k += 1;
		}
	}
}

/*
 * do one quadtree reduction step on array a
 * results put into b (which may be the same as a)
 */
static void
qtree_reduce(a,n,nx,ny,b)
unsigned char a[];
int n;
int nx;
int ny;
unsigned char b[];
{
int i, j, k;
int s10, s00;

	k = 0;							/* k is index of b[i/2,j/2]	*/
	for (i = 0; i<nx-1; i += 2) {
		s00 = n*i;					/* s00 is index of a[i,j]	*/
		s10 = s00+n;				/* s10 is index of a[i+1,j]	*/
		for (j = 0; j<ny-1; j += 2) {
			b[k] =	(a[s10+1] != 0)
				| ( (a[s10  ] != 0) << 1)
				| ( (a[s00+1] != 0) << 2)
				| ( (a[s00  ] != 0) << 3);
			k += 1;
			s00 += 2;
			s10 += 2;
		}
		if (j < ny) {
			/*
			 * row size is odd, do last element in row
			 * s00+1,s10+1 are off edge
			 */
			b[k] =  ( (a[s10  ] != 0) << 1)
				  | ( (a[s00  ] != 0) << 3);
			k += 1;
		}
	}
	if (i < nx) {
		/*
		 * column size is odd, do last row
		 * s10,s10+1 are off edge
		 */
		s00 = n*i;
		for (j = 0; j<ny-1; j += 2) {
			b[k] =  ( (a[s00+1] != 0) << 2)
				  | ( (a[s00  ] != 0) << 3);
			k += 1;
			s00 += 2;
		}
		if (j < ny) {
			/*
			 * both row and column size are odd, do corner element
			 * s00+1, s10, s10+1 are off edge
			 */
			b[k] = ( (a[s00  ] != 0) << 3);
			k += 1;
		}
	}
}

static void
write_bdirect(outfile,a,n,nqx,nqy,scratch,bit)
FILE *outfile;
int a[];
int n;
int nqx,nqy;
unsigned char scratch[];
int bit;
{
int i;

	/*
	 * Write the direct bitmap warning code
	 */
	output_nybble(outfile,0x0);
	/*
	 * Copy A to scratch array (again!), packing 4 bits/nybble
	 */
	qtree_onebit(a,n,nqx,nqy,scratch,bit);
	/*
	 * write to outfile
	 */
	for (i = 0; i < ((nqx+1)/2) * ((nqy+1)/2); i++) {
		output_nybble(outfile,scratch[i]);
	}
}
