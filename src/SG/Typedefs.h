#pragma once

#include <memory>

namespace SG {

class Box;
class Camera;
class Cylinder;
class Ellipsoid;
class Image;
class LayoutOptions;
class Node;
class Object;
class Polygon;
class Rectangle;
class Resource;
class Sampler;
class Scene;
class ShaderProgram;
class ShaderSource;
class Shape;
class StateTable;
class TextNode;
class Texture;
class Uniform;
class UniformDef;

//! \file
//! This file contains convenience typedefs; each is for a shared_ptr to some
//! type of scene graph object.

typedef std::shared_ptr<Box>           BoxPtr;
typedef std::shared_ptr<Camera>        CameraPtr;
typedef std::shared_ptr<Cylinder>      CylinderPtr;
typedef std::shared_ptr<Ellipsoid>     EllipsoidPtr;
typedef std::shared_ptr<Image>         ImagePtr;
typedef std::shared_ptr<LayoutOptions> LayoutOptionsPtr;
typedef std::shared_ptr<Node>          NodePtr;
typedef std::shared_ptr<Object>        ObjectPtr;
typedef std::shared_ptr<Polygon>       PolygonPtr;
typedef std::shared_ptr<Rectangle>     RectanglePtr;
typedef std::shared_ptr<Resource>      ResourcePtr;
typedef std::shared_ptr<Sampler>       SamplerPtr;
typedef std::shared_ptr<Scene>         ScenePtr;
typedef std::shared_ptr<ShaderProgram> ShaderProgramPtr;
typedef std::shared_ptr<ShaderSource>  ShaderSourcePtr;
typedef std::shared_ptr<Shape>         ShapePtr;
typedef std::shared_ptr<StateTable>    StateTablePtr;
typedef std::shared_ptr<TextNode>      TextNodePtr;
typedef std::shared_ptr<Texture>       TexturePtr;
typedef std::shared_ptr<Uniform>       UniformPtr;
typedef std::shared_ptr<UniformDef>    UniformDefPtr;

}  // namespace SG
