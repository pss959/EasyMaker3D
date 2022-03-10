#!/usr/bin/env bash

# Fixes a PNG icon image file that uses grayscale instead of sRGB.

#-----------------------------------------------------------------------------
# Prints a usage message and exits.
#-----------------------------------------------------------------------------
function print_usage() {
    echo "Usage: fixpng.sh <imagefile.png> ..."
    exit
}

#-----------------------------------------------------------------------------
# Mainline.
#-----------------------------------------------------------------------------

if [[ $# -lt 1 ]] ; then print_usage ; fi

for png_file in $*
do
    echo "Updating $png_file"
    convert -define colorspace:auto-grayscale=off "$png_file" "PNG32:$png_file"
done
