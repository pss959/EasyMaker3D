# Application Design Overview {#Overview}

This section describes some of the principles and decisions in the design of
$(APP_NAME). Reading the User Guide in the [user
documentation](https://pss959.github.io/EasyMaker3D/latest) first
would likely be very helpful.

## Module Layers

The source code is divided into modules, each of which has a similarly-named
subdirectory. The modules are arranged (roughly) in a hierarchy:

@dotfile moduledependencies.dot

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

## Software Layers

Ion

SG

Everything else?

## Development and Debugging Aids

Tuning.h/cpp

Logging

Ion remote
