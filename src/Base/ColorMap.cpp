#include "Managers/ColorManager.h"

#include <random>

#include <ion/math/angleutils.h>

#include "Math/Linear.h"
#include "Util/Assert.h"
#include "Util/General.h"

std::unordered_map<std::string, Color> ColorManager::special_map_;

ColorManager::ColorManager() {
    // Create a random number generator with a constant seed for repeatability.
    std::default_random_engine            gen(0x12345678);
    std::uniform_real_distribution<float> dist(0, 1);

    const int kModelColorCount = 12;

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

        const float h = Lerp(dist(gen), min_hue, max_hue);
        const float s = Lerp(dist(gen),
                             kMinModelSaturation_, kMaxModelSaturation_);
        const float v = Lerp(dist(gen), kMinModelValue_, kMaxModelValue_);
        model_colors_.push_back(Color::FromHSV(h, s, v));
    }
}

void ColorManager::Reset() {
    ClearSpecialColors();
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
        return GetSpecialColor("DimensionColorX");
    else if (dim == 1)
        return GetSpecialColor("DimensionColorY");
    else
        return GetSpecialColor("DimensionColorZ");
}

void ColorManager::AddSpecialColor(const std::string &name,
                                   const Color &color) {
    ASSERTM(! Util::MapContains(special_map_, name),
            "Duplicate special color: " + name);
    special_map_[name] = color;
}

Color ColorManager::GetSpecialColor(const std::string &name) {
    ASSERTM(Util::MapContains(special_map_, name),
            "Missing special color: " + name);
    return special_map_[name];
}

void ColorManager::ClearSpecialColors() {
    special_map_.clear();
}
