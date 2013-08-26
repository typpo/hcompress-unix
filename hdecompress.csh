#! /bin/csh
# Shell file to do H-transform image de-compression for a list of files.
#
# If .H is not explicitly specified on end of compressed file names,
# it is added.
#
# Files are replaced by uncompressed file with .H removed from name.
#
# Copyright (c) 1993 Association of Universities for Research
# in Astronomy. All rights reserved. Produced under National
# Aeronautics and Space Administration Contract No. NAS5-26555.
#
set noclobber
set shellfile=$0
set cdir=`dirname $shellfile`
set prgnam=${shellfile:t}
#
if ($#argv == 0) then
	echo "Usage: ${prgnam} [options] files... [options] files..."
	echo "  where options are:"
	echo "     -s to enable smoothing"
	echo "     -u to disable smoothing (default)"
	echo "     -k to keep the compressed file (default)"
	echo "     -r to remove the compressed file"
	echo "     -o raw | net | fits | hhh to specify the output image format"
	echo "        The default output format is raw (= hhh) unless the original"
	echo "        image was FITS format, in which case the default is fits"
	echo "        format."
	exit
endif
set remove=0
set smooth
set format
set nextformat=0
foreach file ($*)
	if ($nextformat) then
		set format=$file
		set nextformat=0
	else if ("$file" == "-o") then
		set nextformat=1
	else if ("$file" == "-r") then
		set remove=1
	else if ("$file" == "-k") then
		set remove=0
	else if ("$file" == "-s") then
		set smooth="-s"
	else if ("$file" == "-u") then
		set smooth
	else
		if (${file:e} == 'H') then
			set infile=$file
			set outfile=${file:r}
		else
			set infile=$file.H
			set outfile=$file
		endif
		if (-e $outfile) then
			echo "${prgnam}: ${outfile} already exists"
		else if (! -e $infile) then
			echo "${prgnam}: ${infile}: No such file"
		else
			echo -n "$outfile "
			if ($format == "") then
				$cdir/hdecomp -v $smooth < $infile > $outfile
			else
				$cdir/hdecomp -v $smooth -o $format < $infile > $outfile
			endif
			if ($status == 0) then
				if ($remove) then
					# delete compressed file
					/bin/rm $infile
				endif
			else
				echo "${prgnam}: error, $infile not decompressed
				/bin/rm $outfile
			endif
		endif
	endif
end
