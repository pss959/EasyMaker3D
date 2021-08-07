# IMakerVR

A test for implementing MakerVR using the Ion library + OpenXR.

## Dependencies

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

Input may be partitioned into files that can be included in other files. Here
is an example:

``` python
Obj {
    children: [
        INCLUDE "some/file_path",
        INCLUDE "some/other/file_path",
    ]
}
```

Note that this means it would be a bad idea to have an object type named
`"INCLUDE"`.
