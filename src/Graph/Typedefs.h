#pragma once

#include <memory>

namespace Graph {

class Cylinder;
class Image;
class Node;
class Object;
class Resource;
class Sampler;
class Scene;
class ShaderProgram;
class ShaderSource;
class Shape;
class Texture;

//! \file This file contains convenience typedefs; each is for a shared_ptr to
//! some type of Graph object.
//!
//! \ingroup Graph

typedef std::shared_ptr<Cylinder>      CylinderPtr;
typedef std::shared_ptr<Image>         ImagePtr;
typedef std::shared_ptr<Node>          NodePtr;
typedef std::shared_ptr<Object>        ObjectPtr;
typedef std::shared_ptr<Resource>      ResourcePtr;
typedef std::shared_ptr<Sampler>       SamplerPtr;
typedef std::shared_ptr<Scene>         ScenePtr;
typedef std::shared_ptr<ShaderProgram> ShaderProgramPtr;
typedef std::shared_ptr<ShaderSource>  ShaderSourcePtr;
typedef std::shared_ptr<Shape>         ShapePtr;
typedef std::shared_ptr<Texture>       TexturePtr;

}  // namespace Graph
