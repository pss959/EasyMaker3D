#pragma once

#include <memory>

namespace SG {

class Box;
class Camera;
class Cylinder;
class Ellipsoid;
class Image;
class Interactor;
class ImportedShape;
class LayoutOptions;
class Line;
class Material;
class Node;
class Object;
class PointLight;
class Polygon;
class Rectangle;
class RenderPass;
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
typedef std::shared_ptr<Box>           BoxPtr;
typedef std::shared_ptr<Camera>        CameraPtr;
typedef std::shared_ptr<Cylinder>      CylinderPtr;
typedef std::shared_ptr<Ellipsoid>     EllipsoidPtr;
typedef std::shared_ptr<Image>         ImagePtr;
typedef std::shared_ptr<ImportedShape> ImportedShapePtr;
typedef std::shared_ptr<Interactor>    InteractorPtr;
typedef std::shared_ptr<LayoutOptions> LayoutOptionsPtr;
typedef std::shared_ptr<Line>          LinePtr;
typedef std::shared_ptr<Material>      MaterialPtr;
typedef std::shared_ptr<Node>          NodePtr;
typedef std::shared_ptr<Object>        ObjectPtr;
typedef std::shared_ptr<PointLight>    PointLightPtr;
typedef std::shared_ptr<Polygon>       PolygonPtr;
typedef std::shared_ptr<Rectangle>     RectanglePtr;
typedef std::shared_ptr<RenderPass>    RenderPassPtr;
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
