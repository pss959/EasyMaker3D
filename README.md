# IMakerVR

A test for implementing MakerVR using the Ion library + OpenXR.

## Setup

TODO: Fill this in

> On Ubuntu 20.10, some combination of NVidia drivers, Vulkan drivers, and
> SteamVR makes SteamVR not recognize the Vive headset as a VR display. The
> only way so far I found to fix this is:

  `cd /usr/share/vulkan/icd.d ; sudo mv intel_icd.x86_64.json intel_icd.x86_64.json.disabled`

> Note that this must be done each time after the `mesa-vulkan-drivers` package
> is installed or updated.

### Windows

TODO: Clean this up

Using MinGW toolchain on Windows. Install MinGW on your system and set the
`MINGW_PATH` environment variable to the location of the installed MinGW
directory.

Install `msys2` and use it to install:

git
mingw-w64-x86_64-gcc
mingw-w64-x86_64-scons
mingw-w64-x86_64-pk-conf

+ Dependencies listed below:

mingw-w64-x86_64-boost
mingw-w64-x86_64-cgal
mingw-w64-x86_64-freetype
mingw-w64-x86_64-glfw
mingw-w64-x86_64-jsoncpp
mingw-w64-x86_64-minizip
mingw-w64-x86_64-openxr-sdk
mingw-w64-x86_64-stb
mingw-w64-x86_64-tinyxml2

#### NOTE

pacman's version of stb did not have an `stb.pc` file, so create one with these
contents:

```
prefix=/mingw64
includedir=${prefix}/include/stb

Name: stb
Description: single-file public domain (or MIT licensed) libraries for C/C++
Version: r1943.c9064e3-2
```

and copy it to `/mingw64/lib/pkgconfig/stb.pc`

### Dependencies

+ `magic_enum` (header only; included as a submodule)
+ `freetype2`
+ `glfw3`
+ `jsoncpp`
+ `libjpeg`
+ `minizip`
+ `openxr`
+ `stb`
+ `tinyxml2`
+ `zlib`

On a Linux system, install the following packages:

`libfreetype-dev libglfw3-dev libjsoncpp-dev libjpeg-dev libminizip-dev libopenx
r-dev libstb-dev libtinyxml2-dev libz-dev`

The `googletest` and `libboost-dev` packages are required to run the unit
tests.

## File Format

Various parts of the application read data from files. The parser that reads
these files creates a generic data structure, which is returned to the caller
so it can then validate the semantics and use the results.

### Basic Syntax

An input file contains a single object. An object is of the form:

```
TypeName ["name"] {
   <comma-separated fields>
}
```

The quoted name is optional, but must be supplied if the object is to be
referred to later.

Each field is of the form `name: value`, where the value format depends on the
field type. Expected types are supplied to the parser by the caller.

If a field stores multiple objects (type `kObjects`), the objects are enclosed
in square brackets and are separated by commas. For example:

``` python
SomeObj {
   children: [
      SomeObj { ... },
      ADifferentObj { ...},
   ]
}
```

A comma after the last field or sub-object is ignored.

### Comments

Comments begin with the `'#'` character and continue until the end of the line.

### Constants

If the first non-whitespace character in an object is a `'['`, the
bracket-enclosed contents define constants that can be used within the rest of
the object or any nested objects. A constant value is always a double-quoted
string whose contents are substituted when the constant is referred to.  The
closest enclosing object scope is used if multiple constants with the same name
are defined.

A constant is referred to with a dollar sign followed by the name. This token
is substituted with the string used to define the constant as if the contents
appeared in the same place (plain text substitution). Note that this
substitution is recursive, so constants can refer to other constants, even with
forward references.

Note that a constant reference can appear only where whitespace can occur.

Example:

``` python
Obj "A" {
    [
        POINT: "3 2.4",
        CHILD: "Obj { foo: 16 }"
    ]
    point2: $POINT,
    point3: $POINT 4,
    children: [$CHILD],
}

```

The constants block may be followed by an optional comma.

### References

Wherever an object may appear, a reference to a previously-named object may be
used. For example:

``` python
Obj {
    children: [
        Obj {
           children: [
              Obj "NamedObj" {  # Defining "NamedObj" here.
                 ...
              }
           ],
        }
        Obj {
           children: [
              Obj "NamedObj";   # This is a reference to it.
           ],
        }
    ],
}
```

The reference specification ends with a semicolon instead of a definition in
curly braces.

### Including Files

Input may be partitioned into files that can be included in other files. Paths
may be absolute or relative to the base path supplied to the parser. Here is an
example of the syntax:

``` python
Obj {
    children: [
        <some/file_path>,
        <some/other/file_path>,
    ]
}
```

Note that an included file must contain one object.

## Graph Namespace

The classes in the Graph namespace define objects that form a scene graph. Each
Graph object wraps some sort of Ion object or objects. The Graph classes
provide an API for interacting with the objects (which Ion does not) and hide a
lot of implementation details.

