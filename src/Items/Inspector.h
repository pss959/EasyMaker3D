#pragma once

#include <functional>

#include "Base/Memory.h"
#include "Math/Types.h"
#include "SG/Node.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(Controller);
DECL_SHARED_PTR(Inspector);

/// The Inspector class is used for interactive inspection of an object. When
/// in VR, the selected Model is attached to one of the controllers for
/// viewing.  When not in VR, the selected Model is shown front and center and
/// can be rotated by moving the mouse.
///
/// \ingroup Items
class Inspector : public SG::Node {
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

    virtual void PostSetUpIon() override;

  protected:
    Inspector() {}

  private:
    std::function<void()> deactivation_func_;

    /// Node that is used to transform the inspected Node or to attach to the
    /// controller.
    SG::NodePtr   transformer_;

    /// Node that is used to scale the inspected Node to a reasonable size.
    SG::NodePtr   scaler_;

    /// Node that is used to center the inspected Node on the origin. The Node
    /// is added as a child of this.
    SG::NodePtr   centerer_;

    /// Controller attached to; may be null.
    ControllerPtr attached_controller_;

    friend class Parser::Registry;
};
