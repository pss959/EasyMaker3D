#include "Managers/ColorManager.h"

#include <random>

#include "Math/Linear.h"
#include "Util/Assert.h"
#include "Util/General.h"

std::unordered_map<std::string, Color> ColorManager::special_map_;

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
    special_map_.clear();
    next_model_index_ = 0;
}

const Color & ColorManager::GetNextModelColor() {
    const Color &color = model_colors_[next_model_index_];
    next_model_index_ = (next_model_index_ + 1) % model_colors_.size();
    return color;
}

Color ColorManager::GetColorForDimension(int dim) {
    ASSERT(dim >= 0 && dim <= 2);
    if (dim == 0)
        return Color(.821f, .132f, .132f);
    else if (dim == 1)
        return Color(.058f, .689f, .058f);
    else
        return Color(.056f, .336f, .849f);
}

void ColorManager::AddSpecialColor(const std::string &name,
                                   const Color &color) {
    ASSERT(! Util::MapContains(special_map_, name));
    special_map_[name] = color;
}

Color ColorManager::GetSpecialColor(const std::string &name) {
    ASSERTM(Util::MapContains(special_map_, name),
            "Missing special color: " + name);
    return special_map_[name];
}
