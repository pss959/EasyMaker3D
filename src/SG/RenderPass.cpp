#include "SG/RenderPass.h"

#include <assert.h>

#include <ion/gfx/node.h>
#include <ion/gfx/shaderinputregistry.h>

#include "SG/Math.h"
#include "SG/Node.h"
#include "SG/SpecBuilder.h"

namespace SG {

Parser::ObjectSpec RenderPass::GetObjectSpec() {
    SG::SpecBuilder<RenderPass> builder;
    builder.AddObject<Node>("root", &RenderPass::root_);
    // This is abstract, so cannot create an instance.
    return Parser::ObjectSpec{
        "RenderPass", false, nullptr, builder.GetSpecs() };
}

const ion::gfx::NodePtr & RenderPass::GetIonRoot() const {
    ASSERT(GetRootNode());
    ASSERT(GetRootNode()->GetIonNode());
    return GetRootNode()->GetIonNode();
}

void RenderPass::InitGlobalUniforms() {
    const ion::gfx::NodePtr &root = GetIonRoot();

    auto &reg = ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
    const Matrix4f id4 = Matrix4f::Identity();
    root->AddUniform(reg->Create<ion::gfx::Uniform>("uProjectionMatrix", id4));
    root->AddUniform(reg->Create<ion::gfx::Uniform>("uModelviewMatrix",  id4));
    root->AddUniform(reg->Create<ion::gfx::Uniform>("uViewportSize",
                                                    Vector2i(0, 0)));
}

}  // namespace SG
