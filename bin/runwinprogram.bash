#!/usr/bin/env bash

echo "=== Running '$*'"

mode=rel
if [[ $# -gt 0 ]] ; then
   mode=$1
   shift
fi

bin_dir="./build/$mode"
ion_dir="./build/$mode/ionsrc"
ovr_dir="./submodules/openvr/bin/win64"

env PATH="$PATH:$bin_dir:$ion_dir:$ovr_dir" $*
