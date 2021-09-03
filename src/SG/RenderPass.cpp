#include "SG/RenderPass.h"

#include <ion/gfx/node.h>

#include "Assert.h"
#include "SG/Node.h"

namespace SG {

void RenderPass::AddFields() {
    AddField(root_);
}

const ion::gfx::NodePtr & RenderPass::GetIonRoot() const {
    ASSERT(GetRootNode());
    ASSERT(GetRootNode()->GetIonNode());
    return GetRootNode()->GetIonNode();
}

}  // namespace SG
