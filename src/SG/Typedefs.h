#pragma once

#include <memory>

namespace SG {

/// \file
/// This file contains convenience typedefs; each is for a shared_ptr to some
/// type of scene graph object.

#define SET_UP_CLASS_(CL) class CL; typedef std::shared_ptr<CL> CL ## Ptr

SET_UP_CLASS_(Box);
SET_UP_CLASS_(Camera);
SET_UP_CLASS_(Cylinder);
SET_UP_CLASS_(Ellipsoid);
SET_UP_CLASS_(FileImage);
SET_UP_CLASS_(Gantry);
SET_UP_CLASS_(Image);
SET_UP_CLASS_(ImportedShape);
SET_UP_CLASS_(LayoutOptions);
SET_UP_CLASS_(Line);
SET_UP_CLASS_(Material);
SET_UP_CLASS_(Node);
SET_UP_CLASS_(Object);
SET_UP_CLASS_(PointLight);
SET_UP_CLASS_(PolyLine);
SET_UP_CLASS_(Polygon);
SET_UP_CLASS_(ProceduralImage);
SET_UP_CLASS_(Rectangle);
SET_UP_CLASS_(RenderPass);
SET_UP_CLASS_(Resource);
SET_UP_CLASS_(Sampler);
SET_UP_CLASS_(Scene);
SET_UP_CLASS_(ShaderNode);
SET_UP_CLASS_(ShaderProgram);
SET_UP_CLASS_(ShaderSource);
SET_UP_CLASS_(Shape);
SET_UP_CLASS_(StateTable);
SET_UP_CLASS_(TextNode);
SET_UP_CLASS_(Texture);
SET_UP_CLASS_(Torus);
SET_UP_CLASS_(Uniform);
SET_UP_CLASS_(UniformBlock);
SET_UP_CLASS_(UniformDef);
SET_UP_CLASS_(VRCamera);
SET_UP_CLASS_(WindowCamera);

#undef SET_UP_CLASS_

}  // namespace SG
