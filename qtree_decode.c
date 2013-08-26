/* Copyright (c) 1993 Association of Universities for Research 
 * in Astronomy. All rights reserved. Produced under National   
 * Aeronautics and Space Administration Contract No. NAS5-26555.
 */
/* qtree_decode.c	Read stream of codes from infile and construct bit planes
 *					in quadrant of 2-D array using binary quadtree coding
 *
 * Programmer: R. White		Date: 7 May 1991
 */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define input_nybble(infile)    input_nbits(infile,4)

extern int  input_nbits();
extern int  input_bit();

static void qtree_expand();
static void qtree_bitins();
static void qtree_copy();
static void read_bdirect();
static int  input_huffman();

extern void
qtree_decode(infile,a,n,nqx,nqy,nbitplanes)
FILE *infile;
int a[];							/* a is 2-D array with dimensions (n,n)	*/
int n;								/* length of full row in a				*/
int nqx;							/* partial length of row to decode		*/
int nqy;							/* partial length of column (<=n)		*/
int nbitplanes;						/* number of bitplanes to decode		*/
{
int log2n, k, bit, b, nqmax;
int nx,ny,nfx,nfy,c;
int nqx2, nqy2;
unsigned char *scratch;

	/*
	 * log2n is log2 of max(nqx,nqy) rounded up to next power of 2
	 */
	nqmax = (nqx>nqy) ? nqx : nqy;
	log2n = log((float) nqmax)/log(2.0)+0.5;
	if (nqmax > (1<<log2n)) {
		log2n += 1;
	}
	/*
	 * allocate scratch array for working space
	 */
	nqx2=(nqx+1)/2;
	nqy2=(nqy+1)/2;
	scratch = (unsigned char *) malloc(nqx2*nqy2);
	if (scratch == (unsigned char *) NULL) {
		fprintf(stderr, "qtree_decode: insufficient memory\n");
		exit(-1);
	}
	/*
	 * now decode each bit plane, starting at the top
	 * A is assumed to be initialized to zero
	 */
	for (bit = nbitplanes-1; bit >= 0; bit--) {
		/*
		 * Was bitplane was quadtree-coded or written directly?
		 */
		b = input_nybble(infile);
		if(b == 0) {
			/*
			 * bit map was written directly
			 */
			read_bdirect(infile,a,n,nqx,nqy,scratch,bit);
		} else if (b != 0xf) {
			fprintf(stderr, "qtree_decode: bad format code %x\n",b);
			exit(-1);
		} else {
			/*
			 * bitmap was quadtree-coded, do log2n expansions
			 *
			 * read first code
			 */
			scratch[0] = input_huffman(infile);
			/*
			 * now do log2n expansions, reading codes from file as necessary
			 */
			nx = 1;
			ny = 1;
			nfx = nqx;
			nfy = nqy;
			c = 1<<log2n;
			for (k = 1; k<log2n; k++) {
				/*
				 * this somewhat cryptic code generates the sequence
				 * n[k-1] = (n[k]+1)/2 where n[log2n]=nqx or nqy
				 */
				c = c>>1;
				nx = nx<<1;
				ny = ny<<1;
				if (nfx <= c) { nx -= 1; } else { nfx -= c; }
				if (nfy <= c) { ny -= 1; } else { nfy -= c; }
				qtree_expand(infile,scratch,nx,ny,scratch);
			}
			/*
			 * now copy last set of 4-bit codes to bitplane bit of array a
			 */
			qtree_bitins(scratch,nqx,nqy,a,n,bit);
		}
	}
	free(scratch);
}


/*
 * do one quadtree expansion step on array a[(nqx+1)/2,(nqy+1)/2]
 * results put into b[nqx,nqy] (which may be the same as a)
 */
static void
qtree_expand(infile,a,nx,ny,b)
FILE *infile;
unsigned char a[];
int nx;
int ny;
unsigned char b[];
{
int i;

	/*
	 * first copy a to b, expanding each 4-bit value
	 */
	qtree_copy(a,nx,ny,b,ny);
	/*
	 * now read new 4-bit values into b for each non-zero element
	 */
	for (i = nx*ny-1; i >= 0; i--) {
		if (b[i] != 0) b[i] = input_huffman(infile);
	}
}

/*
 * copy 4-bit values from a[(nx+1)/2,(ny+1)/2] to b[nx,ny], expanding
 * each value to 2x2 pixels
 * a,b may be same array
 */
static void
qtree_copy(a,nx,ny,b,n)
unsigned char a[];
int nx;
int ny;
unsigned char b[];
int n;		/* declared y dimension of b */
{
int i, j, k, nx2, ny2;
int s00, s10;

	/*
	 * first copy 4-bit values to b
	 * start at end in case a,b are same array
	 */
	nx2 = (nx+1)/2;
	ny2 = (ny+1)/2;
	k = ny2*(nx2-1)+ny2-1;			/* k   is index of a[i,j]			*/
	for (i = nx2-1; i >= 0; i--) {
		s00 = 2*(n*i+ny2-1);		/* s00 is index of b[2*i,2*j]		*/
		for (j = ny2-1; j >= 0; j--) {
			b[s00] = a[k];
			k -= 1;
			s00 -= 2;
		}
	}
	/*
	 * now expand each 2x2 block
	 */
	for (i = 0; i<nx-1; i += 2) {
		s00 = n*i;					/* s00 is index of b[i,j]	*/
		s10 = s00+n;				/* s10 is index of b[i+1,j]	*/
		for (j = 0; j<ny-1; j += 2) {
			b[s10+1] =  b[s00]     & 1;
			b[s10  ] = (b[s00]>>1) & 1;
			b[s00+1] = (b[s00]>>2) & 1;
			b[s00  ] = (b[s00]>>3) & 1;
			s00 += 2;
			s10 += 2;
		}
		if (j < ny) {
			/*
			 * row size is odd, do last element in row
			 * s00+1, s10+1 are off edge
			 */
			b[s10  ] = (b[s00]>>1) & 1;
			b[s00  ] = (b[s00]>>3) & 1;
		}
	}
	if (i < nx) {
		/*
		 * column size is odd, do last row
		 * s10, s10+1 are off edge
		 */
		s00 = n*i;
		for (j = 0; j<ny-1; j += 2) {
			b[s00+1] = (b[s00]>>2) & 1;
			b[s00  ] = (b[s00]>>3) & 1;
			s00 += 2;
		}
		if (j < ny) {
			/*
			 * both row and column size are odd, do corner element
			 * s00+1, s10, s10+1 are off edge
			 */
			b[s00  ] = (b[s00]>>3) & 1;
		}
	}
}

/*
 * Copy 4-bit values from a[(nx+1)/2,(ny+1)/2] to b[nx,ny], expanding
 * each value to 2x2 pixels and inserting into bitplane BIT of B.
 * A,B may NOT be same array (it wouldn't make sense to be inserting
 * bits into the same array anyway.)
 */
static void
qtree_bitins(a,nx,ny,b,n,bit)
unsigned char a[];
int nx;
int ny;
int b[];
int n;		/* declared y dimension of b */
int bit;
{
int i, j, k;
int s00, s10;

	/*
	 * expand each 2x2 block
	 */
	k = 0;							/* k   is index of a[i/2,j/2]	*/
	for (i = 0; i<nx-1; i += 2) {
		s00 = n*i;					/* s00 is index of b[i,j]		*/
		s10 = s00+n;				/* s10 is index of b[i+1,j]		*/
		for (j = 0; j<ny-1; j += 2) {
			b[s10+1] |= ( a[k]     & 1) << bit;
			b[s10  ] |= ((a[k]>>1) & 1) << bit;
			b[s00+1] |= ((a[k]>>2) & 1) << bit;
			b[s00  ] |= ((a[k]>>3) & 1) << bit;
			s00 += 2;
			s10 += 2;
			k += 1;
		}
		if (j < ny) {
			/*
			 * row size is odd, do last element in row
			 * s00+1, s10+1 are off edge
			 */
			b[s10  ] |= ((a[k]>>1) & 1) << bit;
			b[s00  ] |= ((a[k]>>3) & 1) << bit;
			k += 1;
		}
	}
	if (i < nx) {
		/*
		 * column size is odd, do last row
		 * s10, s10+1 are off edge
		 */
		s00 = n*i;
		for (j = 0; j<ny-1; j += 2) {
			b[s00+1] |= ((a[k]>>2) & 1) << bit;
			b[s00  ] |= ((a[k]>>3) & 1) << bit;
			s00 += 2;
			k += 1;
		}
		if (j < ny) {
			/*
			 * both row and column size are odd, do corner element
			 * s00+1, s10, s10+1 are off edge
			 */
			b[s00  ] |= ((a[k]>>3) & 1) << bit;
			k += 1;
		}
	}
}

static void
read_bdirect(infile,a,n,nqx,nqy,scratch,bit)
FILE *infile;
int a[];
int n;
int nqx;
int nqy;
unsigned char scratch[];
int bit;
{
int i;

	/*
	 * read bit image packed 4 pixels/nybble
	 */
	for (i = 0; i < ((nqx+1)/2) * ((nqy+1)/2); i++) {
		scratch[i] = input_nybble(infile);
	}
	/*
	 * insert in bitplane BIT of image A
	 */
	qtree_bitins(scratch,nqx,nqy,a,n,bit);
}

/*
 * Huffman decoding for fixed codes
 *
 * Coded values range from 0-15
 *
 * Huffman code values (hex):
 *
 *	3e, 00, 01, 08, 02, 09, 1a, 1b,
 *	03, 1c, 0a, 1d, 0b, 1e, 3f, 0c
 *
 * and number of bits in each code:
 *
 *	6,  3,  3,  4,  3,  4,  5,  5,
 *	3,  5,  4,  5,  4,  5,  6,  4
 */
static int
input_huffman(infile)
FILE *infile;
{
int c;

	/*
	 * get first 3 bits to start
	 */
	c = input_nbits(infile,3);
	if (c < 4) {
		/*
		 * this is all we need
		 * return 1,2,4,8 for c=0,1,2,3
		 */
		return(1<<c);
	}
	/*
	 * get the next bit
	 */
	c = input_bit(infile) | (c<<1);
	if (c < 13) {
		/*
		 * OK, 4 bits is enough
		 */
		switch (c) {
			case  8 : return(3);
			case  9 : return(5);
			case 10 : return(10);
			case 11 : return(12);
			case 12 : return(15);
		}
	}
	/*
	 * get yet another bit
	 */
	c = input_bit(infile) | (c<<1);
	if (c < 31) {
		/*
		 * OK, 5 bits is enough
		 */
		switch (c) {
			case 26 : return(6);
			case 27 : return(7);
			case 28 : return(9);
			case 29 : return(11);
			case 30 : return(13);
		}
	}
	/*
	 * need the 6th bit
	 */
	c = input_bit(infile) | (c<<1);
	if (c == 62) {
		return(0);
	} else {
		return(14);
	}
}
