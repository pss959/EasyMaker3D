#pragma once

#include <ion/gfx/node.h>

#include "Camera.h"

//! A Scene struct represents a read-in scene.
struct Scene {
  public:
    const std::string  path;    //! Path to the scene's resource file.
    ion::gfx::NodePtr  root;    //! Root node of the scene.
    Camera             camera;  //! Camera for the scene.

    //! The constructor is passed the name of the resource path that the scene
    //! is read from.
    Scene(const std::string &path_in) : path(path_in),
                                        root(new ion::gfx::Node) {}
};
