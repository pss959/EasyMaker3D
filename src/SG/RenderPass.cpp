#include "SG/RenderPass.h"

#include <assert.h>

#include "SG/Node.h"
#include "SG/SpecBuilder.h"

namespace SG {

Parser::ObjectSpec RenderPass::GetObjectSpec() {
    SG::SpecBuilder<RenderPass> builder;
    builder.AddObject<Node>("root", &RenderPass::root_);
    // This is abstract, so cannot create an instance.
    return Parser::ObjectSpec{
        // XXXX"RenderPass", false, []{ return nullptr; }, builder.GetSpecs() };
        "RenderPass", false, nullptr, builder.GetSpecs() };
}

const ion::gfx::NodePtr & RenderPass::GetIonRoot() const {
    ASSERT(GetRootNode());
    ASSERT(GetRootNode()->GetIonNode());
    return GetRootNode()->GetIonNode();
}


}  // namespace SG
