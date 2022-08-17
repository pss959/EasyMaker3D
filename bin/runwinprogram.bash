#!/usr/bin/env bash

echo "=== Running '$*'"

mode=rel
if [[ $# -gt 0 ]] ; then
   mode=$1
   shift
fi

bin_dir="./build/$mode"
ovr_dir="./submodules/openvr/bin/win64"

env PATH="$PATH:$bin_dir:$ovr_dir" $*
