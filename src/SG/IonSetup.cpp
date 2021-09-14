#include "SG/IonSetup.h"

#include <stack>

#include "Assert.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "SG/Scene.h"
#include "SG/Shape.h"
#include "SG/Visitor.h"

namespace SG {

using ion::gfx::ShaderInputRegistry;
using ion::gfx::ShaderInputRegistryPtr;
using ion::gfxutils::ShaderManager;
using ion::text::FontManager;

// ----------------------------------------------------------------------------
// Phase1Visitor_ class.
// ----------------------------------------------------------------------------

// The Phase1Visitor_ class implements the first phase of Ion setup: creating
// and storing ShaderInputRegistry and ShaderProgram instances.
class Phase1Visitor_ : public Visitor {
  public:
    Phase1Visitor_(ShaderManager &shader_manager) :
        shader_manager_(shader_manager) {}

    //! Sets up the given Scene.
    void SetUpScene(Scene &scene);

    //! Sets up for the given RenderPass.
    void SetUpPass(RenderPass &pass) {
        // Start with the global registry on the stack.
        reg_stack_.push(ShaderInputRegistry::GetGlobalRegistry());
        Visit(pass.GetRootNode());
    }

  protected:
    // Visitor traversal functions.
    virtual TraversalCode VisitNodeStart(const NodePath &path) override;
    virtual void            VisitNodeEnd(const NodePath &path) override;

  private:
    ShaderManager &shader_manager_;

    //! This keeps track of the current ShaderInputRegistry during traversal.
    std::stack<ShaderInputRegistryPtr> reg_stack_;
};

void Phase1Visitor_::SetUpScene(Scene &scene) {
    // Start with the global registry on the stack.
    reg_stack_.push(ShaderInputRegistry::GetGlobalRegistry());

    // Set up for each RenderPass.
    for (const auto &pass: scene.GetRenderPasses())
        Visit(pass->GetRootNode());

    ASSERT(reg_stack_.size() == 1U);
}

Visitor::TraversalCode Phase1Visitor_::VisitNodeStart(
    const NodePath &path) {
    Node &node = *path.back();

    std::cerr << "XXXX Visiting " << node.GetDesc() << "\n";
    if (auto &prog = node.GetShaderProgram()) {
        // XXXX
        std::cerr << "XXXX === Found " << prog->GetDesc() << "\n";
    }

    return Visitor::TraversalCode::kContinue;
}

void Phase1Visitor_::VisitNodeEnd(const NodePath &path) {
}

// ----------------------------------------------------------------------------
// IonSetup functions.
// ----------------------------------------------------------------------------

void IonSetup::SetUpScene(Scene &scene, Tracker &tracker,
                          ShaderManager &shader_manager,
                          FontManager &font_manager) {
    // Setup is divided into phases because some objects require knowledge
    // about multiple render passes to operate correctly.

    // The first phase sets up Ion ShaderInputRegistry and ShaderProgram
    // instances.
    Phase1Visitor_ visitor1(shader_manager);
    visitor1.SetUpScene(scene);
}

}  // namespace SG
