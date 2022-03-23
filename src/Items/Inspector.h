#pragma once

#include <memory>

#include "Items/Grippable.h"
#include "Math/Types.h"
#include "SG/Node.h"

namespace Parser { class Registry; }

/// The Inspector class is used for interactive inspection of an object. When
/// in VR, the selected Model is attached to one of the controllers for
/// viewing.  When not in VR, the selected Model is shown front and center and
/// can be rotated by moving the mouse.
class Inspector : public Grippable {
  public:
    /// Activates the Inspector to view the given Node.
    void Activate(const SG::NodePtr &node);

    /// Deactivates the Inspector.
    void Deactivate();

    /// Applies a relative change to the scale of the inspected object.
    void ApplyScaleChange(float delta);

    /// Applies a rotation to the inspected object.
    void ApplyRotation(const Rotationf &rot);

    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual bool IsGrippableEnabled() const override;
    virtual void UpdateGripInfo(GripInfo &info) override;
    virtual void ActivateGrip(Hand hand, bool is_active) override;

  protected:
    Inspector() {}

  private:
    /// Saves default translation while active.
    Vector3f saved_translation_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<Inspector> InspectorPtr;
