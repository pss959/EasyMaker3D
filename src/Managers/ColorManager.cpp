#include "Managers/ColorManager.h"

#include <random>

#include "Math/Linear.h"

ColorManager::ColorManager() {
    // Create a random number generator with a constant seed for repeatability.
    std::default_random_engine            gen(0x12345678);
    std::uniform_real_distribution<float> dist(0, 1);

    // Constants defining Model colors.
    const int   kModelColorCount = 12;
    const float kMinSaturation = .25f;
    const float kMaxSaturation = .4f;
    const float kMinValue      = .9f;
    const float kMaxValue      = .95f;

    // Create the Model colors, alternating among 5 different hue ranges so
    // that consecutive colors are never too close.
    model_colors_.reserve(kModelColorCount);
    const int kNumHueRanges = 5;
    const float hue_range_size = 1 / static_cast<float>(kNumHueRanges);
    int hue_count = 0;
    for (int i = 0; i < kModelColorCount; ++i) {
        const float min_hue = hue_range_size * (hue_count % kNumHueRanges);
        const float max_hue = min_hue + hue_range_size;
        hue_count += 3;  // 0, 3, 1, 4, 2, ...

        const float h = Lerp(dist(gen), min_hue,        max_hue);
        const float s = Lerp(dist(gen), kMinSaturation, kMaxSaturation);
        const float v = Lerp(dist(gen), kMinValue,      kMaxValue);
        model_colors_.push_back(Color::FromHSV(h, s, v));
    }
}

void ColorManager::Reset() {
    next_model_index_ = 0;
}

const Color & ColorManager::GetNextModelColor() {
    const Color &color = model_colors_[next_model_index_];
    next_model_index_ = (next_model_index_ + 1) % model_colors_.size();
    return color;
}
