#pragma once

#include <memory>

namespace Graph {

class Image;
class Node;
class Resource;
class Scene;
class ShaderSource;
class Shape;

//! \file This file contains convenience typedefs; each is for a shared_ptr to
//! some type of Graph object.
//!
//! \ingroup Graph

typedef std::shared_ptr<Image>        ImagePtr;
typedef std::shared_ptr<Node>         NodePtr;
typedef std::shared_ptr<Resource>     ResourcePtr;
typedef std::shared_ptr<Scene>        ScenePtr;
typedef std::shared_ptr<ShaderSource> ShaderSourcePtr;
typedef std::shared_ptr<Shape>        ShapePtr;

}  // namespace Graph
