#!/usr/bin/env bash

# Reverts all PublicDoc images that have not changed very much.
# Uses the custom "imagemetric" tool in git, which runs ImageMagick:
#     compare -fuzz 4% -metric AE $local $remote NULL:

trap '{ echo "QUITTING" ; exit 1; }' INT

cd /home/pss/git/EasyMaker3D

# Error tolerance for images to be considered close enough.
declare -i tolerance
tolerance=400

# The diff-filter arg should ignore new images, but it does not seem to work.
git dt --diff-filter=M -t imagemetric ./PublicDoc/docs/images 2>&1 |
while read line
do
    words=( $line )
    error=${words[0]}
    filename=${words[1]}
    if (( $error < $tolerance )) ; then
        echo "Reverting $filename"
        git checkout HEAD -- $filename
    fi
done


