#include "Parser/Registry.h"
#include "SG/Box.h"
#include "SG/ColorMap.h"
#include "SG/Cylinder.h"
#include "SG/Ellipsoid.h"
#include "SG/FileImage.h"
#include "SG/Gantry.h"
#include "SG/ImportedShape.h"
#include "SG/LayoutOptions.h"
#include "SG/LightingPass.h"
#include "SG/Line.h"
#include "SG/Material.h"
#include "SG/MutableTriMeshShape.h"
#include "SG/NamedColor.h"
#include "SG/Node.h"
#include "SG/PointLight.h"
#include "SG/PolyLine.h"
#include "SG/Polygon.h"
#include "SG/ProceduralImage.h"
#include "SG/Rectangle.h"
#include "SG/RegularPolygon.h"
#include "SG/Sampler.h"
#include "SG/Scene.h"
#include "SG/ShaderProgram.h"
#include "SG/ShaderSource.h"
#include "SG/ShadowPass.h"
#include "SG/StateTable.h"
#include "SG/SubImage.h"
#include "SG/TextNode.h"
#include "SG/Texture.h"
#include "SG/Torus.h"
#include "SG/Tube.h"
#include "SG/Uniform.h"
#include "SG/UniformBlock.h"
#include "SG/UniformDef.h"
#include "SG/UnscopedNode.h"
#include "SG/VRCamera.h"
#include "SG/WindowCamera.h"

#define REGISTER_SG_TYPE_(T) Parser::Registry::AddType<SG::T>(#T)

void RegisterSGTypes_();

void RegisterSGTypes_() {
    REGISTER_SG_TYPE_(Box);
    REGISTER_SG_TYPE_(ColorMap);
    REGISTER_SG_TYPE_(Cylinder);
    REGISTER_SG_TYPE_(Ellipsoid);
    REGISTER_SG_TYPE_(FileImage);
    REGISTER_SG_TYPE_(Gantry);
    REGISTER_SG_TYPE_(ImportedShape);
    REGISTER_SG_TYPE_(LayoutOptions);
    REGISTER_SG_TYPE_(LightingPass);
    REGISTER_SG_TYPE_(Line);
    REGISTER_SG_TYPE_(Material);
    REGISTER_SG_TYPE_(MutableTriMeshShape);
    REGISTER_SG_TYPE_(NamedColor);
    REGISTER_SG_TYPE_(Node);
    REGISTER_SG_TYPE_(PointLight);
    REGISTER_SG_TYPE_(PolyLine);
    REGISTER_SG_TYPE_(Polygon);
    REGISTER_SG_TYPE_(ProceduralImage);
    REGISTER_SG_TYPE_(Rectangle);
    REGISTER_SG_TYPE_(RegularPolygon);
    REGISTER_SG_TYPE_(Sampler);
    REGISTER_SG_TYPE_(Scene);
    REGISTER_SG_TYPE_(ShaderProgram);
    REGISTER_SG_TYPE_(ShaderSource);
    REGISTER_SG_TYPE_(ShadowPass);
    REGISTER_SG_TYPE_(StateTable);
    REGISTER_SG_TYPE_(SubImage);
    REGISTER_SG_TYPE_(TextNode);
    REGISTER_SG_TYPE_(Texture);
    REGISTER_SG_TYPE_(Torus);
    REGISTER_SG_TYPE_(Tube);
    REGISTER_SG_TYPE_(Uniform);
    REGISTER_SG_TYPE_(UniformBlock);
    REGISTER_SG_TYPE_(UniformDef);
    REGISTER_SG_TYPE_(UnscopedNode);
    REGISTER_SG_TYPE_(VRCamera);
    REGISTER_SG_TYPE_(WindowCamera);
}

#undef REGISTER_SG_TYPE_

