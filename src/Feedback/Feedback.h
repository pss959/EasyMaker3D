#pragma once

#include "Math/Types.h"
#include "SG/Node.h"
#include "Util/Assert.h"
#include "Util/Memory.h"

struct Color;
DECL_SHARED_PTR(Feedback);

/// Feedback is an abstract base class for objects that display graphical and
/// text feedback during interactive operations.
///
/// \ingroup Feedback
class Feedback : public SG::Node {
  public:
    /// Alias for function returning scene bounds.
    using SceneBoundsFunc = std::function<Bounds()>;

    /// Sets a function that returns the Bounds of all Models in the
    /// scene. This is used to help determine feedback placement.
    void SetSceneBoundsFunc(const SceneBoundsFunc &func) {
        scene_bounds_func_ = func;
    }

    /// Sets a matrix converting from object to world coordinates.
    void SetObjectToWorldMatrix(const Matrix4f &owm) { object_to_world_ = owm; }

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

    /// Returns a matrix converting from object to world coordinates.
    const Matrix4f & GetObjectToWorldMatrix() const { return object_to_world_; }

  private:
    SceneBoundsFunc scene_bounds_func_;
    Matrix4f        object_to_world_ = Matrix4f::Identity();
};
