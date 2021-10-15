#!/bin/bash

#
# Builds debug and optimized versions of Ion, both static and shared. Installs
# them in /local/inst/ion
#

trap '{ echo "*** EXITING" ; exit 1; }' INT

# Static versions.
pushd ion
echo "======== Building static dbg version"
./build.sh -c dbg
echo "======== Building static opt version"
./build.sh -c opt
echo "======== Installing static dbg version"
./install.sh dbg
echo "======== Installing static opt version"
./install.sh opt
popd

# Shared version.
echo "======== Building shared dbg version"
scons -j8 --mode=dbg --brief
echo "======== Building shared opt version"
scons -j8 --mode=opt --brief
echo "======== Installing shared versions"
cp build/opt/libionshared.so /local/inst/ion/opt/lib/
cp build/dbg/libionshared.so /local/inst/ion/dbg/lib/
