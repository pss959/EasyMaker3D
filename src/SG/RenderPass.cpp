#include "SG/RenderPass.h"

#include "SG/SpecBuilder.h"
#include "SG/Node.h"
#include "SG/ShaderProgram.h"

namespace SG {

Parser::ObjectSpec RenderPass::GetObjectSpec() {
    SG::SpecBuilder<RenderPass> builder;
    builder.AddEnum<Type>("type",              &RenderPass::type_);
    builder.AddObject<ShaderProgram>("shader", &RenderPass::shader_);
    builder.AddObject<Node>("root",            &RenderPass::root_);
    return Parser::ObjectSpec{
        "RenderPass", false, []{ return new RenderPass; }, builder.GetSpecs() };
}

}  // namespace SG
