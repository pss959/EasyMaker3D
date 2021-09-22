﻿#include "Managers/AnimationManager.h"

#include <algorithm>
#include <functional>

#include "Util/General.h"

void AnimationManager::StartAnimation(const AnimationFunc &func) {
    // Add the function to the list.
    AnimData_ data;
    data.func        = func;
    data.start_time  = Util::Time::Now();
    data.is_finished = false;
    anim_data_.push_back(data);
}

bool AnimationManager::ProcessUpdate() {
    bool animating = ! anim_data_.empty();

    if (animating) {
        Util::Time now = Util::Time::Now();

        for (auto &data: anim_data_) {
            // Invoke the function and see if it finished.
            const float time =
                static_cast<float>(now.SecondsSince(data.start_time));
            data.is_finished = ! data.func(time);
        }

        // Remove any finished animations.
        Util::EraseIf(anim_data_,
                      [](const AnimData_ &d){ return d.is_finished; });

        animating = ! anim_data_.empty();
    }
    return animating;
}
