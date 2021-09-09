#include "SG/RenderPass.h"

#include <ion/gfx/node.h>

#include "Assert.h"
#include "SG/Node.h"
#include "SG/Visitor.h"

namespace SG {

// ----------------------------------------------------------------------------
// RenderPass::Updater_ class.
// ----------------------------------------------------------------------------

//! Derived Visitor class that is used to update all Nodes in the graph for
//! rendering.
class RenderPass::Updater_ : public Visitor {
  public:
    //! The constructor is passed the PassType of the RenderPass. UniformBlock
    //! instances are updated based on this.
    Updater_(const PassType type) : pass_type_(type) {}

  protected:
    virtual TraversalCode VisitNodeStart(const NodePath &path) override {
        path.back()->UpdateForRenderPass(pass_type_);
        return Visitor::TraversalCode::kContinue;
    }
  private:
    const PassType pass_type_;
};

// ----------------------------------------------------------------------------
// RenderPass functions.
// ----------------------------------------------------------------------------

void RenderPass::AddFields() {
    AddField(root_);
}

const ion::gfx::NodePtr & RenderPass::GetIonRoot() const {
    ASSERT(GetRootNode());
    ASSERT(GetRootNode()->GetIonNode());
    return GetRootNode()->GetIonNode();
}

void RenderPass::UpdateForRender() {
    Updater_ updater(GetPassType());
    updater.Visit(GetRootNode());
}

}  // namespace SG
