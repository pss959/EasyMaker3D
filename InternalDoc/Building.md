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
the Github repository. They are:

<table class="markdownTable">
    <tr>
       <td>[`docopt.cpp`](https://github.com/docopt/docopt.cpp)</td>
       <td>A port of `docopt` to C++, used for command-line processing.</td>
    </tr>
    <tr>
       <td>[`googletest`](https://github.com/google/googletest)</td>
       <td>Used for unit testing.</td>
    </tr>
    <tr>
       <td>[`magic_enum`](https://github.com/Neargye/magic_enum)</td>
       <td>Adds a bunch of very useful C++ enum-related functions (that really
           should be built into the language IMO).</td>
    </tr>
    <tr>
       <td>[`openvr`](https://github.com/ValveSoftware/openvr)</td>
       <td>Valve's SDK for VR devices.</td>
    </tr>
</table>

In addition, there is a stripped down and modified version of the [Google
Ion](https://github.com/google/ion) source that provides rendering and other
features in the `ionsrc` subdirectory.

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

 - The [`Pillow`](https://python-pillow.org/) Python package (`PIL`
   replacement) is required to build the 2D radial menu icon resource files
   (required for the application).
 - The [`Sphinx`](https://www.sphinx-doc.org/),
   [`sphinx-rtd-theme`](https://sphinx-rtd-theme.readthedocs.io), and
   [`sphinxcontrib-jquery`](https://pypi.org/project/sphinxcontrib-jquery/)
   Python packages are required to build the public documentation.
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

    boost cgal gcc glfw3 jsoncpp libjpeg minizip tinyxml2

You may need to do this:

    cp /usr/local/Cellar/jpeg/9e/lib/pkgconfig/libjpeg.pc /usr/local/lib/pkgconfig

You may also need to use `brew` to install the `freeglut` package if there are
build errors related to `OpenGL`.

## Build Modes {#BuildModes}

There are four different modes you can specify when building the application:

  - **rel** (release) is used to build the final release package, which is a
    Zip file on Windows and Linux and a DMG on the Mac.
  - **opt** (optimized) is the default mode.
  - **dbg** (debug) adds debugging information to the executable. It is slower
    and takes longer to build, but is very handy when fixing issues.
  - **cov** (coverage) is used to build and run [unit tests](#UnitTests) with
    coverage tracking enabled.
    
The `opt` and `dbg` modes include some features that help when developing the
application. These features are compiled out in `rel` mode.

To change which mode is being used, specify the `mode` option when building.
For example, to build the debugging version:

    scons --mode=dbg ...

The build is set up to print brief versions of what it is doing, such as
`"Compiling foo.cpp"`.  If you want to see each command line in full,
excruciating detail, pass the `--nobrief` option to `scons`.

## Build Targets {#BuildTargets}

The targets consist of three main groups:

  - **Applications**, including
    + _$(APP_NAME)_: The main application.
    + _createcheatsheet_: Creates the Cheat Sheet reStructuredText file used in
      the public documentation.
    + _nodeviewer_: An interactive application that can help view individual
      models for convenience.
    + _printtypes_: Writes to standard output all known concrete parsable types
      with their fields. (See [Parser Objects](#Parser::Object).)
    + _printtypes_: Writes to standard output all known concrete parsable types
      with their fields. (See [Parser Objects](#Parser::Object).)
    + _snapimage_: Used to create most images for the public documentation.

  - **Documentation**, both internal (what you are reading) and external
    (uploaded to Github Pages).

  - **Tests**, to build and run unit tests using the [Google
    Test](https://github.com/google/googletest) framework.

`SCons` can take any individual file (full path) as a target, or any of the
following aliases:

| Target          | Results                                               |
|:---------------:|:------------------------------------------------------|
| **App**         | Builds the $(APP_NAME) application                    |
| **Apps**        | Builds all applications                               |
| **Coverage**    | Builds and runs unit tests, generates coverage report |
| **Icons**       | Builds the 2D menu icon image resources               |
| **Images**      | Builds all images for public documentation            |
| **InternalDoc** | Builds the internal documentation you are reading     |
| **PublicDoc**   | Builds the public documentation                       |
| **Release**     | Builds the release zip or DMG file (`rel` mode only)  |
| **RunTests**    | Builds and runs unit tests                            |
| **Tests**       | Builds unit tests                                     |

Note that some targets are defined only in the Linux build. This can be changed
fairly easily in the `SCons` files if necessary.

## Examples

All of these examples assume you run the command in the top-level $(APP_NAME)
directory.

Note that the `-j8` option passed to `scons` allows 8 parallel jobs to run to
speed things up; adjust this number for your particular build host.

### Release

To build the release:

    scons -j8 --mode=rel Release

This creates a file containing the release (Zip or DMG) in the
`build\rel\Release\` directory. This is all that is needed when releasing a new
version of the application (along with updating the public documentation as
needed).

### Application(s)

To build the main application in the default `opt` mode:

    scons -j8 App
    
The result is <tt>build/opt/Apps/$(APP_NAME)</tt>.

    scons -j8 --mode=dbg App
    
builds the main application in `dbg` mode. The result is
<tt>build/dbg/Apps/$(APP_NAME)</tt>.

To build the main application and all [auxiliary applications](#BuildTargets):

    scons -j8 Apps

### Object Files

This example shows how to build a single object file (`Util/String.os`, built
from `Util/String.cpp`) with output containing the full command used to compile
it. This can be handy to make sure all compiler arguments are correct.

    scons build/opt/Util/String.os --nobrief
    
### Tests

To build all unit tests in `opt` mode:

    scons -j8 Tests

To build and run all unit tests in `opt` mode:

    scons -j8 RunTests

There are two ways to limit the set of tests that are run with the `RunTests`
target:

    scons -j8 RunTests TESTFILTER="*Bevel*"

runs all unit tests that have the string "Bevel" in their names.

    scons -j8 RunTests TESTCATEGORIES=Math,Util

runs all unit tests that are in the `Math` and `Util` categories. The
categories correspond to [code Modules](#Modules), which also correspond to
code subdirectories.

There is an additional `Timing` category that runs tests with performance
experiments. These tests are disabled by default in `src/Tests/TestMain.cpp`
and have to be enabled specifically with the `TESTFILTER` or `TESTCATEGORIES`
options.

It is sometimes useful to enable [key-based runtime logging](#KLogging) for
unit tests. The `KLOG` option does this:

    scons -j8 RunTests TESTFILTER="MainHandler*" KLOG="h"

### Code Coverage

The `cov` build mode enables code coverage tracking. To see code coverage for
all tests, run:

    scons -j8 --mode=cov Coverage

This produces an HTML coverage report in `build/cov/coverage/index.html`.

The `TESTFILTER` and `TESTCATEGORIES` options described above also work for
coverage, so this command:

    scons -j8 --mode=cov Coverage TESTCATEGORIES=Math
    
will produce a report after running only the `Math` tests; this can be used to
determine if all of those tests cover all the lines of code in the `Math`
module.

### Documentation

To build this internal documentation into `build/InternalDoc/html/`:

    scons -j8 InternalDoc

To build the public documentation into `build/PublicDoc/html/`:

    scons PublicDoc

Note that using more than one parallel job can mess things up when creating the
images and is not recommended.

## Build Input Files

In case you need to modify some aspect of the build, the input files to `SCons`
are as follows:

| File                               | Contents                                                 |
|------------------------------------|----------------------------------------------------------|
| `SConstruct`                       | The main input file                                      |
| `SConscript_env`                   | Sets up `SCons` environments for all modes and platforms |
| `SConscript_release`               | Rules to build the release file                          |
| `InternalDoc/SConscript_dox`       | Rules to build the internal documentation                |
| `PublicDoc/SConscript_doc`         | Rules to build the public documentation                  |
| `PublicDoc/SConscript_snaps`       | Rules to build snapshot images for public documentation  |
| `ionsrc/SConscript_ion`            | Rules to build the `IonLib` library                      |
| `ionsrc/SConscript_iondoc`         | Rules to build internal documentation for Ion            |
| `ionsrc/SConscript_iontests`       | Rules to build and run tests for Ion                     |
| `ionsrc/SConscript_ion`            | Rules to build the `IonLib` library                      |
| `resources/SConscript_resources`   | Rules to build generated resource files                  |
| `src/SConscript_src`               | Rules to build the main library                          |
| `src/Apps/SConscript_apps`         | Rules to build all applications                          |
| `src/Tests/SConscript_tests`       | Rules to build and run all tests                         |
| `submodules/SConscript_submodules` | Rules to build all submodules                            |
| `submodules/SConscript_docopt`     | Rules to build the `docopt` submodule library            |
| `submodules/SConscript_googletest` | Rules to build the `gtest` submodule library             |

## Building a Release {#Release}

Here are the steps required to create a new release. These should all be done
in the `master` branch.

  - Decide whether this is a major, minor, or patch release and update the
    version number in the top `SConstruct` file.

  - Add all public-facing changes to `PublicDoc/docs/ReleaseNotes.rst`. You
    can use 
    
        git log vX.Y.Z..HEAD --oneline
        
    to see the first line of all commits since the `X.Y.Z` release.

  - Make any other necessary changes to `PublicDoc` and `InternalDoc`. Commit
    them and push them to Github.

  - Build on all 3 platforms and upload the resulting release files (zip or
    DMG) to Google Drive.

  - Update the public documentation:
     - Build the documentation with `scons PublicDoc`.
     - If any images are regenerated, use `bin/revertimages.sh` to revert any
       images that did not change enough to warrant committing. This script
       uses ImageMagick to measure image changes.
     - Commit any outstanding changes and push to Github.
     - If this is a patch release, remove the previous version of the public
       documentation. For example, if the new patch is version `3.2.7`,
       remove `build/PublicDoc/3.2.6` if it exists and also use `git` to
       remove `docs/3.2.6`.
     - Run
     
           rsync -vau --exclude=.nojekyll --exclude=.doctrees --exclude=.buildinfo --exclude=_sources build/PublicDoc/ docs/
       
       to update the documentation source used by GitHub Pages.
     - Copy `docs/<current-version>/_static/js/versions.js` to all other
       versions under `docs/`. This guarantees that all versions in GitHub
       Pages have the same entries in their version selectors. This shell
       command can do this for new version X.Y.Z::

           ls -d docs/*/_static/js | grep -v "X.Y.Z" | xargs -n 1 cp docs/X.Y.Z/_static/js/versions.js

     - Commit all changes under `docs/` (including the new version) and push
       to GitHub.
         
  - For a major or minor release, create a new release branch. For example, if
    the release version is `3.4.0`, create the branch named `Release-3.4` from
    the current `master` branch. Push the release branch to GitHub.

  - Create a tag for the release with
  
        git tag -f -a vX.Y.Z -m "Version X.Y.Z"
        
    Push the tag to GitHub.
