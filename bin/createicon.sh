#!/usr/bin/env bash

# Converts an input image with optional transparency into one that can be used
# as an icon texture.

#-----------------------------------------------------------------------------
# Prints a usage message and exits.
#-----------------------------------------------------------------------------
function print_usage() {
    echo "Usage: createicon <inputimage> <outputimage>"
    exit
}

#-----------------------------------------------------------------------------
# Mainline.
#-----------------------------------------------------------------------------

if [[ $# -ne 2 ]] ; then print_usage ; fi

# Adjust this for the desired background color.
bg="#aaaaaa"

input="$1"
output="$2"
toRGB="-define colorspace:auto-grayscale=false -type truecolor"
args="-background $bg -alpha remove -alpha off $toRGB"
convert "$input" $args "$output"
