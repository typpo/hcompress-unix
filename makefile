# makefile for hcompress programs
# R. White, 20 April 1992
#
# Change INSTALLDIR to point to the directory where you want the programs
# installed.  (The default is your own bin directory.)  Then do "make install"
# to install the programs.  A simple "make" will compile and link the programs
# in this directory, but will not install them.
#
# Copyright (c) 1993 Association of Universities for Research
# in Astronomy. All rights reserved. Produced under National
# Aeronautics and Space Administration Contract No. NAS5-26555.
#
#INSTALLDIR = ${HOME}/bin
INSTALLDIR = /usr/local/bin
#
CC=gcc
CFLAGS = -O
LIBABBR = hcomp
LIB = lib${LIBABBR}.a
LDFLAGS = -L. -l${LIBABBR} -lm

all: hcomp hdecomp
	@echo "All programs compiled and linked."

clean :
	-/bin/rm *.o hcomp hdecomp ${LIB}

#
# Install the programs:
# (1) Move the executables to INSTALLDIR
# (2) Copy the shell scripts to INSTALLDIR, removing the ".csh" extension
# (3) Create a link from fdecompress to hdecompress in INSTALLDIR
#
install : hcomp hdecomp
	@/bin/mv hcomp       ${INSTALLDIR}/hcomp
	@/bin/mv hdecomp     ${INSTALLDIR}/hdecomp
	@/bin/cp hcompress.csh     ${INSTALLDIR}/hcompress
	@/bin/cp fcompress.csh     ${INSTALLDIR}/fcompress
	@/bin/cp hdecompress.csh   ${INSTALLDIR}/hdecompress
	@/bin/ln -s ${INSTALLDIR}/hdecompress ${INSTALLDIR}/fdecompress
	@echo
	@echo "All files installed in ${INSTALLDIR}"
	@echo "Type 'rehash' to update path tables"

# library

${LIB}: \
		${LIB}(bit_input.o)			\
		${LIB}(bit_output.o)		\
		${LIB}(decode.o)			\
		${LIB}(digitize.o)			\
		${LIB}(dodecode.o)			\
		${LIB}(doencode.o)			\
		${LIB}(encode.o)			\
		${LIB}(fitspass.o)			\
		${LIB}(fitsread.o)			\
		${LIB}(get_data.o)			\
		${LIB}(getopt.o)			\
		${LIB}(hinv.o)				\
		${LIB}(hsmooth.o)			\
		${LIB}(htrans.o)			\
		${LIB}(makefits.o)			\
		${LIB}(put_data.o)			\
		${LIB}(qread.o)				\
		${LIB}(qtree_decode.o)		\
		${LIB}(qtree_encode.o)		\
		${LIB}(qwrite.o)			\
		${LIB}(swap_bytes.o)		\
		${LIB}(undigitize.o)
	ranlib ${LIB}
	@echo Library is up to date

.PRECIOUS: ${LIB}

# New rules for library entries

.c.a:
	${CC} -c ${CFLAGS} $<
	@ar rv ${LIB} $*.o
	@rm -f $*.o

#
# executables
#
hcomp : hcomp.o ${LIB}
	${CC} ${CFLAGS} hcomp.o -o hcomp ${LDFLAGS}
hdecomp : hdecomp.o ${LIB}
	${CC} ${CFLAGS} hdecomp.o -o hdecomp ${LDFLAGS}
