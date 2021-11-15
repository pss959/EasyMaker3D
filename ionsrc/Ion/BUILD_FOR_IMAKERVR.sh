#!/bin/bash

#
# Builds dbg and opt shared Ion library and installs them in /local/inst/ion
#

trap '{ echo "*** EXITING" ; exit 1; }' INT

echo "======== Building shared dbg version"
scons -j8 --mode=dbg --brief
echo "======== Building shared opt version"
scons -j8 --mode=opt --brief
echo "======== Installing shared versions"
cp build/opt/libionshared.so /local/inst/ion/opt/lib/
cp build/dbg/libionshared.so /local/inst/ion/dbg/lib/
