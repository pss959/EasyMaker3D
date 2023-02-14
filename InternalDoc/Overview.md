# Application Design and Implementation Overview {#Overview}

This section describes some of the overall design of $(APP_NAME). Familiarity
with the material in the User Guide in the [user
documentation](https://pss959.github.io/EasyMaker3D/latest) would likely be
very helpful.

## Design

### Size and Precision

$(APP_NAME) is targeted for casual 3D printing, which places some useful
constraints on the application:

  - Most 3D printers have a relatively small, fixed size, meaning there is no
    need to support an infinite size range.
  - The size of printer nozzles is also constrained, meaning that model
    features smaller than about a tenth of a millimeter are unlikely to be
    useful. This limits the precision of various modeling operations.
    
These constraints are exhibited in the room and stage sizes and in the
available precision settings.

### VR and Multi-Platform

Allowing the application to work in VR and on multiple platforms adds more
requirements:

  - The UI cannot be a standard 2D menu-based one, since that does not work
    well in VR.
  - The UI must also be available on all platforms.

The result is a custom embedded 3D UI that works both with mouse/keyboard and
VR controllers.

### Coordinate Systems

The application uses a conventional 3D graphics coordinate system:
right-handed, with +X to the right, +Y up, and +Z toward the viewer.
Conversions to and from the conventional STL/3D-printing coordinate system (RHS
with +Z up) are performed when necessary. All user-facing documentation and
color coding assume the 3D-printing system.

### Encapsulation

The application relies heavily on encapsulation in its design and
implementation. All data access and modification in classes occurs through
class methods. Similarly, there is a rigid ordering of module dependencies to
avoid any cycles (see the diagram below).

### Session Management

The decision to support infinite undo/redo, even in restored sessions, means
that all changes made to the scene must be saved in files. Each change is an
instance of a derived class of the `Command` class. The `CommandManager` class
stores the full list of commands and supports undo and redo of the entire list.

Session files (with an `.ems` extension) are just another type of [EMD text
resource file](#EMD). The version number is included in the session files in
case the format or contents change in future releases.

## Implementation

### Naming Conventions

The code naming conventions follow the [Google C++ Style
Guide](https://google.github.io/styleguide/cppguide.html) for the most part.
There may be some deviations.

### Namespaces

C++ namespaces are used in a few places: the `SG` module, the `Debug` module,
and some of the `Math` and `Util` modules. There is also a `TK` namespace that
is used for constants used to tune the application.

### Smart Pointers

All objects that can be shared in any way are generally passed around using
shared pointers (`std::shared_ptr`). Classes that support this generally use
the `DECL_SHARED_PTR()` macro to define a shared pointer type. For example, the
`Model` class defines the `ModelPtr` type as a `shared_ptr` to a `Model`
instance.

## Source Code Modules {#Modules}

The source code is divided into modules, each of which has a similarly-named
subdirectory.

| Module    | Description    | Base Class  |
| :-------: | -------------- | ----------- |
| Agents    | \ref Agents    |             |
| App       | \ref App       |             |
| Apps      | \ref Apps      |             |
| Base      | \ref Base      |             |
| Commands  | \ref Commands  | Command     |
| Debug     | \ref Debug     |             |
| Enums     | \ref Enums     |             |
| Executors | \ref Executors | Executor    |
| Feedback  | \ref Feedback  | Feedback    |
| Handlers  | \ref Handlers  | Handler     |
| IO        | \ref IO        |             |
| Items     | \ref Items     |             |
| Managers  | \ref Managers  |             |
| Math      | \ref Math      |             |
| Models    | \ref Models    | Model       |
| Panels    | \ref Panels    | Panel       |
| Panes     | \ref Panes     | Pane        |
| Parser    | \ref Parser    |             |
| Place     | \ref Place     |             |
| SG        | \ref SG        |             |
| Selection | \ref Selection |             |
| Tools     | \ref Tools     | Tool        |
| Trackers  | \ref Trackers  | Tracker     |
| Utility   | \ref Utility   |             |
| VR        | \ref VR        |             |
| Viewers   | \ref Viewers   | Viewer      |
| Widgets   | \ref Widgets   | Widget      |

These modules are arranged (roughly) in a hierarchy shown in this diagram:

@dotfile moduledependencies.dot

The following sections describe some of these modules in more detail, starting
from the innermost.

### Parser: Parsed Objects

The `Parser` module contains a base `Parser::Object` class for all objects that
can be parsed from [EMD resource files](#EMD). This is the base class for all
objects that appear in the scene graph. This scheme allows most of the data for
the application to be read from files at run-time.

Each parsed object may define a set of derived `Parser::Field` instances, each
of which has a name and a specific value type. For example, the `SG::Node`
class described below as a field named `scale` that contains a vector of 3
floats.  The private member variable is named `scale_` and is declared as
follows:

    Parser::TField<Vector3f> scale_;

Each parsed object class redefines the virtual `Parser::Object::AddFields()`
function to add its fields to a new instance. The virtual
`Parser::Object::CreationDone()` function may also be defined to do any work
necessary to set up an instance when it is fully parsed.

The (quasi-singleton) `Parser::Registry` class maintains a registry of all
concrete classes derived from `Parser::Object` that can be parsed from files.
All derived concrete parsable classes have a protected no-argument constructor
and declare `Parser::Registry` as a friend so instances can be created.

### SG: Scene Graph

The `ionsrc` subdirectory contains a fork of the [Google
Ion](https://github.com/google/ion) repository that serves as the underlying
scene graph and rendering library. Ion is a very generic scene graph that maps
to [OpenGL](https://www.opengl.org/) calls. It provides little in the way of
semantics for nodes in the graph, but provides a great abstraction layer for
rendering.

The `SG` module is a higher-level custom scene graph layer that provides
semantics for the application. The base `SG::Object` class is derived from
`Parser::Object` and provides the ability to notify observers (via
`Util::Notifier`) of changes made to it. For example, the `SG::Node` class is
derived from `SG::Object` and tracks changes to its various parts and
descendent nodes.

The Ion and SG scene graphs have unidirectional connections: parent nodes point
to their children and not the other way around. Upwards notification is
implemented using an observer/notifier model (`Util::Notifier`). A reference to
a specific descendent node in a graph or subgraph uses the `SG::NodePath`
class, which allows the entire path of nodes to be examined. For example, ray
intersection with a graph returns an `SG::Hit` instance that includes a path
from the scene root to the intersected node.

#### Scenes

The `SG::Scene` class is a derived `SG::Object` that represents the entire
scene appearing in the application window (and VR headset). It contains the
following items:

  - An `SG::ColorMap` storing named special colors used in the scene.
  - An `SG::Gantry` that holds the derived `SG::Camera` instances (non-VR and
    VR) and allows them to move up and down.
  - A collection of `SG::PointLight` instances used to illuminate the scene.
  - A collection of derived `SG::RenderPass` instances used to render the
    scene. (Typically an `SG::ShadowPass` followed by an `SG::LightingPass`.)
  - The root `SG::Node` of the scene.

#### Nodes

The `SG::Node` class is a derived `SG::Object` that is the basis for the main
scene graph. It contains the following items, all of which are optional:

  - A set of flags indicating which features of the node are disabled
    (rendering, intersection testing, etc.)
  - Fields defining scale, rotation, and translation transformations.
  - Names of any active shaders.
  - Shader uniforms (in `SG::UniformBlock` instances).
  - An `SG::StateTable` defining graphics state.
  - A collection of `SG::Shape` instances defining geometry (see below).
  - A collection of child nodes.
    
Nodes implement a simple inheritance model: anything defined in a node is
inherited by all descendents unless overridden by one of them. The only
exception is transformation values, which accumulate as expected.

Nodes also implement bounding box computation.

#### Shapes

`SG::Shape` is a derived `SG::Object` that defines some sort of geometric
entity to display in a scene graph. There are shape classes for various
primitives and imported meshes. Each shape class defines bound computation and
ray intersection testing.

## Development and Debugging Aids

XXXX

Tuning.h/cpp

Logging

Debug printing/shortcuts

Ion remote
