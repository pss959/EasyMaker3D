# Source Directory Structure {#Structure}

The top-level subdirectories of the $(APP_NAME) project are:

  - **InternalDoc**: Contains the source for the documentation you are reading,
    not including the comments in the code source.

  - **PublicDoc**: Contains the source for the public user documentation. The
    `docs` subdirectory has the
    [reStructuredText](https://docutils.sourceforge.io/rst.html) input for
    generating the doc with [Sphinx](https://www.sphinx-doc.org). The `snaps`
    subdirectory contains scripts, session files, and other data used as input
    to the `snapimage` application for generating the images used in the doc.

  - **bin**: Contains scripts that help build the application and documentation.

  - **build**: Created by the build process; contains all build products,
    including documentation. A subdirectory is created for each [build
    mode](BuildModes).

  - **ionsrc**: Contains a fork of the [Google
    Ion](https://github.com/google/ion) repository with some fixes and
    additions. This is used as the underlying rendering library.

  - **models**: The `controllers` subdirectory contains local versions of the
    SteamVR controller models used for graphical feedback. The `scad`
    subdirectory contains [OpenSCAD](https://openscad.org) input files used to
    create the OFF files for 3D icons.

  - **resources**: Contains all run-time resource files used by the
    application. See the [Resources] page for more details.

  - **src**: Contains the source code for the application. The code is divided
    into [Modules], each of which has a subdirectory. There is also a `Tests`
    subdirectory that contains unit test source code and data.

  - **submodules**: Contains the git submodules the project depends on.
