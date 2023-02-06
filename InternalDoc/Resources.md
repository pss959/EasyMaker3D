# Application Resources and Data Files {#Resources}

## Resource Files

Most of the data used by $(APP_NAME) is read from resource files when it is
run. This includes the contents of the room, the tools, the shaders, and so on.
The `resources` subdirectory contains all of the source data files used by the
application; most of these are packaged into the release file. Many of the data
files are in **EMD** format, described below.

The `resources` directory has the following subdirectories:

| Name            | Contents                                           |
| :-------------: | :------------------------------------------------- |
| `colors`        | EMD file with special application colors           |
| `fonts`         | TrueType font files used by the application        |
| `images`        | Image files used as textures or icons              |
| `json`          | JSON files with SteamVR actions and input bindings |
| `nodes`         | EMD files defining scene graph nodes               |
| `programs`      | EMD files defining shader program attributes       |
| `scenes`        | EMD files defining top-level scenes                |
| `shaders`       | GLSL shader program source files                   |
| `shapes`        | OFF files used for 3D icons and other items        |

## EMD Format

EMD ($(APP_NAME) Data) is designed to be an easy to read and write Python-like
data format.

### Basic Syntax

Comments begin with the `#` character and continue until the end of the line.
All Extra whitespace is ignored, except within quoted strings.

An input file contains a single object. An object is of the form:

~~~~~~~~~~~~~{.py}
TypeName ["name"] {
   <comma-separated fields>
}
~~~~~~~~~~~~~

The quoted name is optional, but must be supplied if the object is to be
referred to later.

Each field is of the form

~~~~~~~~~~~~~{.py}
name: value
~~~~~~~~~~~~~

where the value format depends on the field type. If a field stores multiple
values, they appear within square brackets and are separated by commans.

Some fields contain one or more other objects; the format shown above for an
object is used inside these fields as well, recursively.

### Field Types

A field contains either a single value or a multiple values of a specific
type. The current list of field types is:

| Underlying Type | Format                              | Example                         |
| :-------------: | :---------------------------------- | ------------------------------- |
| bool            | True *or* False                     | True                            |
| float           | value                               | -12.4                           |
| int             | value                               | -16                             |
| unsigned int    | value                               | 27                              |
| string          | quoted string                       | "Example text."                 |
| #Anglef         | degrees                             | 112.5                           |
| #Color          | r g b a *or* "#RRGGBBAA"            | .2 .4 .1 1 *or* "#d34577ff"     |
| #Matrix2f       | <4 floats>                          | 1 0 0 1                         |
| #Matrix3f       | <9 floats>                          | 1 0 0 0 1 0 0 0 1               |
| #Matrix4f       | <16 floats>                         | 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1 |
| #Plane          | distance normal_x normal_y normal_z | .5 0 1 0                        |
| #Point3f        | x y z                               | -1 3.1 4                        |
| #Rotationf      | axis_x axis_y axis_z degrees        | 0 1 0  -90                      |
| #Vector2f       | x y                                 | 1 -2.5                          |
| #Vector2i       | x y                                 | -3 4                            |
| #Vector2ui      | x y                                 | 3 6                             |
| #Vector3f       | x y z                               | 1 -2 .4                         |
| #Vector3i       | x y z                               | 1 -4 6                          |
| #Vector3ui      | x y z                               | 5 2 1                           |
| #Vector4f       | x y z w                             | 1 -2 .4 .2                      |
| #Vector4i       | x y z w                             | -3 1 -4 6                       |
| #Vector4ui      | x y z w                             | 6 4 1 2                         |

Some fields contain one or more other objects. These are typically defined to
hold a specific derived object class.

### Constants

Any object may contain a special field named `CONSTANTS` that defines constant
strings that can be substituted within the scope of that object. This can
provide more consistency and clarity in some cases. 

A constant is referred to with a `$` followed by the name. This token is
substituted with the string used to define the constant as if the contents
appeared in the same place (plain text substitution). Note that this
substitution is recursive, so constants can refer to other constants, even with
forward references. Note that a constant reference can appear only where
whitespace can occur.

For example:

~~~~~~~~~~~~~{.py}
Node {
  CONSTANTS: [
    SIZE: "12",
  ]
  scale: $SIZE $SIZE $SIZE,
  ...
}
~~~~~~~~~~~~~

A constant can contain any string, including an entire field description or
more. For example:

~~~~~~~~~~~~~{.py}
Node {
  CONSTANTS: [
    SCALE: "scale: .3 .4 .5",
  ],
  children: [
    Node "Child0" { 
      $SCALE,
      ...
    },
    Node "Child1" { 
      $SCALE,
      ...
    },
  ]
}
~~~~~~~~~~~~~

Note that the `CONSTANTS` field, if it appears, must appear before any real
fields in the same object.

### Templates

Another special field is named `TEMPLATES`. It is similar to `CONSTANTS`,
except that it defines objects that can be cloned and modified. For example,
this appears in the definition of the #RotationTool object:

~~~~~~~~~~~~~{.py}
RotationTool "RotationTool" {
  TEMPLATES: [
    DiscWidget "T_AxisRotator" {
      mode: "kRotationOnly",
      shapes: [
        Torus "Ring" {
          inner_radius: .4,
          ring_count:   12,
          sector_count: 72,
        }
      ],
    },
  ],
  children: [
    Node {
      rotation: 0 0 1 -90,
      children: [
        CLONE "T_AxisRotator" "XAxisRotator" { inactive_color: $DIM_COLOR_X }
      ],
    },
    ...
  ]
}
~~~~~~~~~~~~~

and two other clones for the Y and Z axes.

Templates are named starting with `"T_"` by convention.

Like the `CONSTANTS` field, the `TEMPLATES` field must appear before any real
fields in the same object.

### Clones and References

As shown in the above template example, a template can be cloned with a `CLONE`
statement. A clone can appear wherever an object of the same type may
appear. The clone, like any other object, may be given a name as in the above
example.

Any object may be cloned, whether or not it is a template. The original field
values defined in the cloned template or object may be overridden inside the
clone.

Another option is the `USE` statement, which inserts a reference to another
object. There is no way to modify the reference; it uses the same object as
is. For example, the `Root.emd` file defines a [Sampler](#SG::Sampler) object
called `DefaultSampler`. This is used as the sampler inside multiple
[Texture](#SG::Texture) objects with the following field:

~~~~~~~~~~~~~{.py}
  sampler: USE "DefaultSampler",
~~~~~~~~~~~~~

### Including Files

Input may be partitioned into files that can be included in other files. Paths
may be absolute or relative to the base path supplied to the parser. Here is an
example of the syntax:

~~~~~~~~~~~~~{.py}
Node {
  children: [
    <"some/file_path.emd">,
    <"some/other/file_path.emd">,
  ]
}
~~~~~~~~~~~~~

Note that an included file must contain one object, and that object must be
valid to use in the including context.
