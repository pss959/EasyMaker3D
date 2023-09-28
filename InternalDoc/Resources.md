# Resource and Data Files {#Resources}

## Resource Files

Most of the data used by $(APP_NAME) is read from resource files when it is
run. This includes the contents of the room, the tools, the shaders, and so on.
The `resources` subdirectory contains all of the source data files used by the
application; most of these are packaged into the release file. Many of the data
files are in **EMD** format, described below.

The `resources` directory has the following subdirectories:

| Subdirectory  | Format   | Contents                           |
| :-----------: | :------: | :--------------------------------- |
| `colors/`     | EMD      | Special application colors         |
| `fonts/`      | TrueType | Fonts used by the application      |
| `images/`     | JPG/PNG  | Images used as textures or icons   |
| `json/`       | JSON     | SteamVR actions and input bindings |
| `nodes/`      | EMD      | Scene graph nodes                  |
| `programs/`   | EMD      | Shader program attributes          |
| `scenes/`     | EMD      | Top-level scenes                   |
| `shaders/`    | GLSL     | Shader program source              |
| `shapes/`     | OFF      | 3D icons and other items           |
\n

## EMD Format {#EMD}

EMD ($(APP_NAME) Data) is designed to be an easy to read and write Python-like
data format.

### Comments and Whitespace

Comments begin with the \CT{'#'} character and continue until the end of the
line.  All extra whitespace is ignored, except within quoted strings. The
formatting used here is solely for readability (and is encouraged).

### Objects

An input file contains a single object. An object is of the form:

<code><pre>
<i>TypeName</i> <i>"name"</i> <b>{</b>
   <i>field1: value,</i>
   <i>field2: value,</i>
   ...
<b>}</b>
</pre></code>

+ The *TypeName* must correspond to the name of a concrete class derived from
  #Parser::Object (most of the classes in the application).

+ The quoted name is optional unless the object is to be referred to later in
  the file or within code.

+ The format of a field's value depends on the type of the field. If a field
  stores multiple values, the values appear within square brackets and are
  separated by commas.

+ A field that contains one or more other objects uses the same format shown
  above for an object, recursively. For example, see the `children` field in
  examples below.

### Field Types

A field contains either a single value or a multiple values of a specific
type. The current list of field types is:

| Underlying Type | Format                                           | Examples                          |
| :-------------: | :----------------------------------------------- | --------------------------------- |
| bool            | **True** or **False**                            | `True`                            |
| float           | *value*                                          | `-12.4`                           |
| int             | *value*                                          | `-16`                             |
| unsigned int    | *value*                                          | `27`                              |
| string          | *quoted string*                                  | `"Example text."`                 |
| #Anglef         | degrees                                          | `112.5`                           |
| #Color          | *r g b a* or <b>\"\#</b><i>RRGGBBAA</i><b>\"</b> | `.2 .4 .1 1` *or* `"#d34577ff"`   |
| #Matrix2f       | *4 floats*                                       | `1 0 0 1`                         |
| #Matrix3f       | *9 floats*                                       | `1 0 0 0 1 0 0 0 1`               |
| #Matrix4f       | *16 floats*                                      | `1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1` |
| #Plane          | *normal_x normal_y normal_z distance*            | `0 1 0 .5`                        |
| #Point3f        | *x y z*                                          | `-1 3.1 4`                        |
| #Rotationf      | *axis_x axis_y axis_z degrees*                   | `0 1 0  -90`                      |
| #Vector2f       | *x y* (float)                                    | `1 -2.5`                          |
| #Vector2i       | *x y* (int)                                      | `-3 4`                            |
| #Vector2ui      | *x y* (unsigned int)                             | `3 6`                             |
| #Vector3f       | *x y z* (float)                                  | `1 -2 .4`                         |
| #Vector3i       | *x y z* (int)                                    | `1 -4 6`                          |
| #Vector3ui      | *x y z* (unsigned int)                           | `5 2 1`                           |
| #Vector4f       | *x y z w* (float)                                | `1 -2 .4 .2`                      |
| #Vector4i       | *x y z w* (int)                                  | `-3 1 -4 6`                       |
| #Vector4ui      | *x y z w* (unsigned int)                         | `6 4 1 2`                         |

Some fields contain one or more other objects. These are typically defined to
hold a specific derived object class.

### Constants

Any object may contain a special field named \CT{CONSTANTS} that defines
constant strings that can be substituted within the scope of that object. This
can provide more consistency and clarity in some cases.

A constant is referred to with a `$` followed by the name. This token is
substituted with the string used to define the constant as if the contents
appeared in the same place (plain text substitution). Note that this
substitution is recursive, so constants can refer to other constants, even with
forward references. Note that a constant reference can appear only where
whitespace can occur.

For example:

<code><pre>
Node {
  CONSTANTS: [
    SIZE: "12",
  ]
  scale: $SIZE $SIZE $SIZE,
  ...
}
</pre></code>

A constant can contain any string, including an entire field description or
more. To embed a double quote in a string, use a backslash. For example:

<code><pre>
Node {
  CONSTANTS: [
    SCALE: "scale: .3 .4 .5",
    TEXT:  "\"Some text\"",
  ],
  children: [
    Node "Child0" { 
      $SCALE,
      ...
    },
    TextNode "Child1" { 
      $SCALE,
      text: $TEXT,
      ...
    },
  ]
}
</pre></code>

Note that the \CT{CONSTANTS} field, if it appears, must appear before any real
fields in the same object.

### Templates

Another special field is named \CT{TEMPLATES}. It is similar to \CT{CONSTANTS},
except that it defines objects that can be cloned and modified. For example,
this appears in the definition of the #RotationTool object:

<code><pre>
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
    Node {
      children: [
        CLONE "T_AxisRotator" "YAxisRotator" { inactive_color: $DIM_COLOR_Y }
      ],
    },
    ...
  ]
}
</pre></code>

and one other clone for the Z axis.

By convention, templates are named starting with CT{"T_"}.

Like the \CT{CONSTANTS} field, the \CT{TEMPLATES} field must appear before any
real fields in the same object.

### Clones and References

As shown in the above template example, a template can be cloned with a
\CT{CLONE} statement. A clone can appear wherever an object of the same type
may appear. The clone, like any other object, may be given a name as in the
above example.

Note that *any* object may be cloned, whether or not it is a template. The
original field values defined in the cloned template or object may be
overridden within the clone; only the clone is affected in this case.

Another option is the \CT{USE} statement, which inserts a reference to another
object. There is no way to modify the reference; it uses the same object as
is. For example, the `Root.emd` file defines a [Sampler](#SG::Sampler) object
called `DefaultSampler`. This is used as the sampler within multiple
[Texture](#SG::Texture) objects with the following field:

<code><pre>
sampler: USE "DefaultSampler",
</pre></code>

### Including Files

Input may be partitioned into files that can be included in other files. Paths
may be absolute or relative to the base path supplied to the parser. Here is an
example of the syntax:

<code><pre>
Node {
  children: [
    <"some/file_path.emd">,
    <"some/other/file_path.emd">,
  ]
}
</pre></code>

Note that an included file must contain one object, and that object must be
valid to use in the including context.
