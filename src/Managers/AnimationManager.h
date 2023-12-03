#pragma once

#include <functional>
#include <vector>

#include "Util/Memory.h"
#include "Util/UTime.h"

DECL_SHARED_PTR(AnimationManager);

/// AnimationManager is responsible for managing animations using specified
/// functions
///
/// \ingroup Managers
class AnimationManager {
  public:
    /// Alias for animation function passed to Animate().
    using AnimationFunc = std::function<bool(float)>;

    /// Indicates that each call to ProcessUpdate() should increment time by
    /// the given amount. Setting to 0 resets to the default behavior, which
    /// uses the actual time.
    void SetFrameIncrement(float seconds) { frame_increment_ = seconds; }

    /// Starts an animation using the given function. The function is passed
    /// the time in seconds since the animation started. It should return true
    /// while the animation is still running and false when it is done.
    void StartAnimation(const AnimationFunc &func);

    /// Updates any running animations. This should be called every frame.
    void ProcessUpdate();

    /// Returns true if anything is currently animating.
    bool IsAnimating() const { return ! anim_data_.empty(); }

  private:
    /// This struct maintains info about a running animation.
    struct AnimData_ {
        AnimationFunc func;         ///< Function used to animate.
        UTime         start_time;   ///< Time the animation was started.
        size_t        frame_count;  ///< Number of animated frames so far.
        bool          is_finished;  ///< Set to true when finished.
    };

    /// Data for currently animating functions.
    std::vector<AnimData_> anim_data_;

    /// Frame time increment; 0 means use the actual time difference.
    float frame_increment_ = 0;
};
