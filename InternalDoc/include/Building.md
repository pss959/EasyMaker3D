# Building the Application {#Building}

$(APP_NAME) is built using the [`SCons` software construction
tool](https://www.scons.org/). `SCons` is one of the dependencies required for
building the application; others are listed below for each platform.

<sub>The author would be happy to discuss why SCons is far superior to
CMake.</sub>

## Dependencies

Although $(APP_NAME) runs on multiple platforms (Windows, Mac, Linux), the
author developed most of it on a Linux system. Note that some parts of the
build (such as generating some resource files and documentation) run only on
Linux, but could easily be adapted to run on other platforms if anyone wants
to.

For your convenience, some dependencies are already included as submodules in
the Github repository.

### Linux

Install the following packages before building the application:

        libfreetype-dev
        libglfw3-dev
        libjpeg-dev
        libjsoncpp-dev
        libminizip-dev
        libstb-dev 
        libtinyxml2-dev
        libz-dev

Additionally:

 - The `Pillow` Python package (`PIL` replacement) is required to build the 2D
   radial menu icon resource files (required for the application).
 - The `Sphinx` and `sphinx-rtd-theme` Python packages are required to build
   the public documentation.
 - The `libboost-dev` package is required to run the unit tests.

### Windows

To make things easier (i.e., more consistent with the Linux build), the Windows
build uses [MSYS2](https://www.msys2.org/). Once installing it, use the
`pacman` command to install the following dependencies:

        mingw-w64-x86_64-brotli
        mingw-w64-x86_64-bzip2
        mingw-w64-x86_64-cgal
        mingw-w64-x86_64-dlfcn
        mingw-w64-x86_64-freetype
        mingw-w64-x86_64-gcc
        mingw-w64-x86_64-glfw
        mingw-w64-x86_64-graphite2
        mingw-w64-x86_64-gtest
        mingw-w64-x86_64-harfbuzz
        mingw-w64-x86_64-jsoncpp
        mingw-w64-x86_64-libjpeg-turbo
        mingw-w64-x86_64-libpng
        mingw-w64-x86_64-minizip
        mingw-w64-x86_64-openxr-sdk
        mingw-w64-x86_64-pkgconf
        mingw-w64-x86_64-python3
        mingw-w64-x86_64-python3-setuptools
        mingw-w64-x86_64-scons
        mingw-w64-x86_64-tinyxml2

Before building, you will need to set the `MSYS2_HOME` environment variable to
the location of the `MSYS2` installation, conveniently accessed as the output
of running `cygpath -am /` in a bash shell.

### Mac

To make things easier (i.e., more consistent with the Linux build), the Mac
build uses [Homebrew](https://brew.sh/). Once installing it, use the `brew`
command to install the following dependencies:

        boost
        cgal
        gcc
        glfw3
        jsoncpp
        libjpeg
        minizip
        tinyxml2

You may need to do this:

        cp /usr/local/Cellar/jpeg/9e/lib/pkgconfig/libjpeg.pc /usr/local/lib/pkgconfig

You may also need to use `brew` to install the `freeglut` package if there are
build errors related to `OpenGL`.

## Build Modes {#BuildModes}

There are three different modes you can specify when building the application:

  - **opt** (optimized) is the default mode.
  - **dbg** (debug) adds debugging information to the executable. It is slower
    and takes longer to build, but is very handy when fixing issues.
  - **rel** (release) is used to build the final release package, which is a
    Zip file on Windows and Linux and a DMG on the Mac.
    
The `opt` and `dbg` modes include some features that help when developing the
application. These features are compiled out in `rel` mode.

To change which mode is being used, specify the `mode` option when building.
For example, to build the debugging version:

        scons --mode=dbg ...

The build is set up to print brief versions of what it is doing, such as
`"Compiling foo.cpp"`.  If you want to see each command line in full,
excruciating detail, pass the `--nobrief` option to `scons`.

## Build Targets

XXXX

## Building the Internal Documentation

The internal application documentation for developers is generated using
the incredible [Doxygen tool](https://www.doxygen.nl/). To generate the
documentation XXXX

## Building the Public Documentation

The internal application documentation for developers is generated using
the incredible [Doxygen tool](https://www.doxygen.nl/). To generate the
documentation 
