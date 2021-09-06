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
    //! The constructor is passed the name of the RenderPass. UniformBlock
    //! instances are updated based on this name.
    Updater_(const std::string &pass_name) : pass_name_(pass_name) {}

  protected:
    virtual TraversalCode VisitNodeStart(const NodePath &path) override {
        path.back()->UpdateForRenderPass(pass_name_);
        return Visitor::TraversalCode::kContinue;
    }
  private:
    const std::string &pass_name_;
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
    Updater_ updater(GetName());
    updater.Visit(GetRootNode());
}

}  // namespace SG
