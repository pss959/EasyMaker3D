#pragma once

#include "Math/Types.h"
#include "Memory.h"
#include "SG/Node.h"
#include "Util/Assert.h"

DECL_SHARED_PTR(Feedback);

/// Feedback is an abstract base class for objects that display graphical and
/// text feedback during interactive operations.
///
/// \ingroup Feedback
class Feedback : public SG::Node {
  public:
    /// Typedef for function returning scene bounds.
    typedef std::function<Bounds()> SceneBoundsFunc;

    /// Sets a function that returns the Bounds of all Models in the
    /// scene. This is used to help determine feedback placement.
    void SetSceneBoundsFunc(const SceneBoundsFunc &func) {
        scene_bounds_func_ = func;
    }

    /// Sets a rotation to use to make text face forward. This rotates text
    /// facing along Z in stage coordinates to do the same in world
    /// coordinates.
    void SetTextRotation(const Rotationf &rot) {
        text_rotation_ = rot;
    }

    /// Returns true if the derived Feedback type is displayed in world
    /// coordinates as opposed to stage coordinates. The base class defines
    /// this to return false.
    virtual bool IsInWorldCoordinates() const { return false; }

    //! Sets the color of an instance.
    virtual void SetColor(const Color &color) = 0;

    /// This is called by the FeedbackManager when an instance is
    /// activated. The base class defines this to do nothing.
    virtual void Activate() {}

    /// This is called by the FeedbackManager when an instance is
    /// deactivated. The base class defines this to do nothing.
    virtual void Deactivate() {}

  protected:
    /// Returns the current scene bounds as computed by the scene bounds
    /// function.
    Bounds GetSceneBounds() {
        ASSERT(scene_bounds_func_);
        return scene_bounds_func_();
    }

    /// Returns a rotation to apply to text that is facing +Z in stage
    /// coordinates so that it faces +Z in world coordinates.
    const Rotationf & GetTextRotation() const { return text_rotation_; }

  private:
    SceneBoundsFunc scene_bounds_func_;
    Rotationf       text_rotation_;
};
