#include "SG/RenderPass.h"

#include "SG/SpecBuilder.h"
#include "SG/Node.h"
#include "SG/ShaderProgram.h"

namespace SG {

void RenderPass::SetUpIon(IonContext &context) {
    if (shader_) {
        shader_->SetUpIon(context);
        context.registry_stack.push(
            shader_->GetIonShaderProgram()->GetRegistry());
    }

    if (root_)
        root_->SetUpIon(context);

    if (shader_) {
        ASSERT(context.registry_stack.top() ==
               shader_->GetIonShaderProgram()->GetRegistry());
        context.registry_stack.pop();
    }
}

Parser::ObjectSpec RenderPass::GetObjectSpec() {
    SG::SpecBuilder<RenderPass> builder;
    builder.AddEnum<Type>("type",              &RenderPass::type_);
    builder.AddObject<ShaderProgram>("shader", &RenderPass::shader_);
    builder.AddObject<Node>("root",            &RenderPass::root_);
    return Parser::ObjectSpec{
        "RenderPass", false, []{ return new RenderPass; }, builder.GetSpecs() };
}

}  // namespace SG
