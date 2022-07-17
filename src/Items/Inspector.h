#pragma once

#include <functional>

#include "Base/Memory.h"
#include "Items/Grippable.h"
#include "Math/Types.h"

namespace Parser { class Registry; }
namespace SG { DECL_SHARED_PTR(Node); }

DECL_SHARED_PTR(Controller);
DECL_SHARED_PTR(Inspector);

/// The Inspector class is used for interactive inspection of an object. When
/// in VR, the selected Model is attached to one of the controllers for
/// viewing.  When not in VR, the selected Model is shown front and center and
/// can be rotated by moving the mouse.
///
/// \ingroup Items
class Inspector : public Grippable {
  public:
    /// Sets a function to invoke when the Inspector is deactivated.
    void SetDeactivationFunc(const std::function<void()> &func) {
        deactivation_func_ = func;
    }

    /// Activates the Inspector to view the given Node. If the Controller is
    /// not null, the node is attached to it. Otherwise, it is attached to the
    /// Inspector itself.
    void Activate(const SG::NodePtr &node, const ControllerPtr &controller);

    /// Deactivates the Inspector.
    void Deactivate();

    /// Applies a relative change to the scale of the inspected object if it is
    /// not attached to a Controller.
    void ApplyScaleChange(float delta);

    /// Applies a rotation to the inspected object if it is not attached to a
    /// Controller.
    void ApplyRotation(const Rotationf &rot);

    /// Sets whether edges are shown for inspected objects.
    void ShowEdges(bool show);

    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual const SG::Node * GetGrippableNode() const override;
    virtual void UpdateGripInfo(GripInfo &info) override;
    virtual void ActivateGrip(Hand hand, bool is_active) override;

  protected:
    Inspector() {}

  private:
    std::function<void()> deactivation_func_;

    /// Node that is used to transform the inspected Node.
    SG::NodePtr   transformer_;

    /// Node that is the parent of the inspected Node.
    SG::NodePtr   parent_;

    /// Node that is used as the parent of the inspected Node when attached to
    /// a Controller.
    SG::NodePtr   parent_for_controller_;

    /// Controller attached to; may be null.
    ControllerPtr attached_controller_;

    friend class Parser::Registry;
};
