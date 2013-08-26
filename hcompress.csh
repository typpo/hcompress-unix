#! /bin/csh
# Shell file to do H-transform image compression for a list of image files
# in .hhh, .hhd format.  *.hhd files are replaced by *.hhd.H.
#
# Program will compress only I*2 images with no group parameters.
# (A warning is issued for other images.)
#
# The default compression scale factor is 666, which is appropriate for
# GASP images (gives about a factor of 10 compression, negligible loss in
# information.)
#
# Copyright (c) 1993 Association of Universities for Research
# in Astronomy. All rights reserved. Produced under National
# Aeronautics and Space Administration Contract No. NAS5-26555.
#
# R. White, 20 April 1992
# 
set noclobber
set shellfile=$0
set cdir=`dirname $shellfile`
set prgnam=${shellfile:t}
#
if ($#argv == 0) then
	echo "Usage: ${prgnam} [options] files... [options] files..."
	echo "  where options are:"
	echo "     -s scale to specify the compression scale factor"
	echo "     -k       to keep the uncompressed file (default)"
	echo "     -r       to remove the uncompressed file"
	echo
	echo "Default compression scale factor is 666 (good for GASP images.)"
	echo "Specify .hhd extension on files.  Compressed files are named *.*.H."
	exit
endif
set scale=666
set nextscale=0
set remove=0
foreach parm ($*)
	if ($nextscale) then
		set scale=$parm
		set nextscale=0
		echo Using scale $scale
	else
		if ("$parm" == "-s") then
			set nextscale=1
		else if ("$parm" == "-r") then
			set remove=1
		else if ("$parm" == "-k") then
			set remove=0
		else
			set datafile="$parm"
			set compfile="${datafile}.H"
			set dext="${datafile:e}"
			set hext="`echo $dext | colrm 3`h"
			set headfile="${datafile:r}.$hext"
			if ("`echo $dext | colrm 1 2`" != "d") then
				echo "${prgnam}: ${datafile} does not have .xxd extension"
			else if (-e $compfile) then
				echo "${prgnam}: $compfile already exists"
			else if (! -e $datafile) then
				echo "${prgnam}: ${datafile}: No such file"
			else if (! -e $headfile) then
				echo "${prgnam}: ${headfile}: No such file"
			else
				#
				# do the compression
				#
				echo -n "$datafile "
				$cdir/hcomp -v -s $scale -i hhh $datafile $headfile \
					> $compfile
				if ($status == 0) then
				  if ($remove) then
				    # delete original file
				    /bin/rm $datafile
				  endif
				else
				  echo "${prgnam}: $datafile not compressed: compression error"
				  /bin/rm $compfile
				endif
			endif
		endif
	endif
end
