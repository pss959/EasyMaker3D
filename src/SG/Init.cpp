#include "SG/Init.h"

#include <ion/gfx/node.h>
#include <ion/gfx/shaderinputregistry.h>

#include "Assert.h"
#include "Parser/Parser.h"
#include "SG/Box.h"
#include "SG/Camera.h"
#include "SG/Cylinder.h"
#include "SG/Ellipsoid.h"
#include "SG/FileImage.h"
#include "SG/ImportedShape.h"
#include "SG/LayoutOptions.h"
#include "SG/LightingPass.h"
#include "SG/Line.h"
#include "SG/Material.h"
#include "SG/Node.h"
#include "SG/PointLight.h"
#include "SG/Polygon.h"
#include "SG/ProceduralImage.h"
#include "SG/Rectangle.h"
#include "SG/Sampler.h"
#include "SG/Scene.h"
#include "SG/ShaderProgram.h"
#include "SG/ShaderSource.h"
#include "SG/ShadowPass.h"
#include "SG/StateTable.h"
#include "SG/TextNode.h"
#include "SG/Texture.h"
#include "SG/Uniform.h"
#include "SG/UniformBlock.h"
#include "SG/UniformDef.h"

using ion::gfx::ShaderInputRegistry;

namespace SG {

static bool s_initialized_ = false;

void Init() {
    if (s_initialized_)
        return;

    s_initialized_ = true;

    // Instead of using the built-in uModelviewMatrix, we use uModelMatrix and
    // uViewMatrix separately. This allows conversions to world coordinates
    // (with just the uModelMatrix). The uViewMatrix should never need to be
    // combined (set once per view). The uModelMatrix is combined the same way
    // uModelviewMatrix is, accumulating during graph traversal. Make sure to
    // do this only once, as it modifies the registry.
    auto &reg = ShaderInputRegistry::GetGlobalRegistry();

    auto *mv_spec = reg->Find<ion::gfx::Uniform>("uModelviewMatrix");
    ASSERT(mv_spec);

    reg->Add(ShaderInputRegistry::UniformSpec(
                 "uModelMatrix", ion::gfx::kMatrix4x4Uniform,
                 "Cumulative model matrix,", mv_spec->combine_function));
    reg->Add(ShaderInputRegistry::UniformSpec(
                 "uViewMatrix", ion::gfx::kMatrix4x4Uniform, "View matrix,"));
}

void RegisterTypes(Parser::Parser &parser) {
#define ADD_TYPE_(T) parser.RegisterObjectType(#T, []{ return new T; });
    ADD_TYPE_(Box);
    ADD_TYPE_(Camera);
    ADD_TYPE_(Cylinder);
    ADD_TYPE_(Ellipsoid);
    ADD_TYPE_(FileImage);
    ADD_TYPE_(ImportedShape);
    ADD_TYPE_(LayoutOptions);
    ADD_TYPE_(LightingPass);
    ADD_TYPE_(Line);
    ADD_TYPE_(Material);
    ADD_TYPE_(Node);
    ADD_TYPE_(PointLight);
    ADD_TYPE_(Polygon);
    ADD_TYPE_(ProceduralImage);
    ADD_TYPE_(Rectangle);
    ADD_TYPE_(Sampler);
    ADD_TYPE_(Scene);
    ADD_TYPE_(ShaderProgram);
    ADD_TYPE_(ShaderSource);
    ADD_TYPE_(ShadowPass);
    ADD_TYPE_(StateTable);
    ADD_TYPE_(TextNode);
    ADD_TYPE_(Texture);
    ADD_TYPE_(Uniform);
    ADD_TYPE_(UniformBlock);
    ADD_TYPE_(UniformDef);
#undef ADD_TYPE_
}

}  // namespace SG
