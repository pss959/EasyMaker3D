# Application Design Overview {#Overview}

This section describes some of the principles and decisions in the design of
$(APP_NAME). Reading the User Guide in the [user
documentation](https://pss959.github.io/EasyMaker3D/latest) first
would likely be very helpful.

## Design Principles

Encapsulation

Run-time resource files

Scene graphs

Coordinate systems/handedness/size/precision

Paths
## Design Constraints

Beginner-focused

VR

Multi-platform

Session files + undo/redo

Data files

## General Comments

### Namespaces

C++ namespaces are used in a few places: the `SG` module, the `Debug` module,
and some of the `Math` and `Util` modules. There is also a `TK` namespace that
is used for constants used to tune the application.

### Naming Conventions

The code naming conventions follow the [Google C++ Style
Guide](https://google.github.io/styleguide/cppguide.html) for the most part.
There may be some deviations.

### Smart Pointers

All objects that can be shared in any way are generally passed around using
shared pointers (`std::shared_ptr`). Classes that support this generally use
the `DECL_SHARED_PTR()` macro to define a shared pointer type. For example, the
`Model` class defines the `ModelPtr` type as a `shared_ptr` to a `Model`
instance.

## Module Layers

The source code is divided into [modules](#Modules), each of which has a
similarly-named subdirectory. The modules are arranged (roughly) in a hierarchy
shown in this diagram:

@dotfile moduledependencies.dot

The following sections describe some of these modules in more detail, starting
from the innermost.

### Parser: Parsed Objects

The `Parser` module contains a base `Parser::Object` class for all objects that
can be parsed from [EMD resource files](#Resources). This is the base class for
all objects that appear in the scene graph. This scheme allows most of the data
for the application to be read from files at run-time.

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

Tuning.h/cpp

Logging

Debug printing/shortcuts

Ion remote
