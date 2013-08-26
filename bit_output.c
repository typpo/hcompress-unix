/* Copyright (c) 1993 Association of Universities for Research 
 * in Astronomy. All rights reserved. Produced under National   
 * Aeronautics and Space Administration Contract No. NAS5-26555.
 */
/* BIT OUTPUT ROUTINES */

#include <stdio.h>

int bitcount;

/* THE BIT BUFFER */

static int buffer;				/* Bits buffered for output					*/
static int bits_to_go;			/* Number of bits free in buffer			*/


/* INITIALIZE FOR BIT OUTPUT */

extern void
start_outputing_bits()
{
	buffer = 0;									/* Buffer is empty to start	*/
	bits_to_go = 8;								/* with						*/
	bitcount = 0;
}


/* OUTPUT A BIT */

extern void
output_bit(outfile,bit)
FILE *outfile;
int bit;
{
	buffer <<= 1;								/* Put bit at end of buffer */
	if (bit) buffer |= 1;
	bits_to_go -= 1;
	if (bits_to_go == 0) {						/* Output buffer if it is	*/
		putc(buffer & 0xff,outfile);			/* now full					*/
		bits_to_go = 8;
		buffer = 0;
	}
	bitcount += 1;
}


/* OUTPUT N BITS (N must be <= 8) */

extern void
output_nbits(outfile,bits,n)
FILE *outfile;
int bits;
int n;
{
	/*
	 * insert bits at end of buffer
	 */
	buffer <<= n;
	buffer |= ( bits & ((1<<n)-1) );
	bits_to_go -= n;
	if (bits_to_go <= 0) {
		/*
		 * buffer full, put out top 8 bits
		 */
		putc((buffer>>(-bits_to_go)) & 0xff,outfile);
		bits_to_go += 8;
	}
	bitcount += n;
}


/* FLUSH OUT THE LAST BITS */

extern void
done_outputing_bits(outfile)
FILE *outfile;
{
	if(bits_to_go < 8) {
		putc(buffer<<bits_to_go,outfile);
		/* count the garbage bits too */
		bitcount += bits_to_go;
	}
}
