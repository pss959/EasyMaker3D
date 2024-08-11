//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Managers/AnimationManager.h"

void AnimationManager::StartAnimation(const AnimationFunc &func) {
    // Add the function to the list.
    AnimData_ data;
    data.func        = func;
    data.start_time  = UTime::Now();
    data.frame_count = 0;
    data.is_finished = false;
    anim_data_.push_back(data);
}

void AnimationManager::ProcessUpdate() {
    if (IsAnimating()) {
        UTime now = UTime::Now();

        for (auto &data: anim_data_) {
            // Compute the elapsed time using actual time or the frame
            // increment.
            const float time = frame_increment_ > 0 ?
                ++data.frame_count * frame_increment_ :
                static_cast<float>(now.SecondsSince(data.start_time));

            // Invoke the function and see if it finished.
            data.is_finished = ! data.func(time);
        }

        // Remove any finished animations.
        std::erase_if(anim_data_,
                      [](const AnimData_ &d){ return d.is_finished; });
    }
}
