#pragma once

#include <functional>

#include "Math/Types.h"
#include "SG/Node.h"
#include "Util/Assert.h"

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

  private:
    SceneBoundsFunc scene_bounds_func_;
};

typedef std::shared_ptr<Feedback> FeedbackPtr;
