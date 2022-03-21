#include "Managers/ColorManager.h"

#include <random>

#include <ion/math/angleutils.h>

#include "Math/Linear.h"
#include "Util/Assert.h"
#include "Util/General.h"

// ----------------------------------------------------------------------------
// Constants.
// ----------------------------------------------------------------------------

namespace {

static const float kRingInnerRadius_    = .45f;
static const float kRingOuterRadius_    = 1;

static const float kMinModelSaturation_ = .25f;
static const float kMaxModelSaturation_ = .40f;
static const float kMinModelValue_      = .90f;
static const float kMaxModelValue_      = .95f;

}  // anonymous namespace

// ----------------------------------------------------------------------------
// ColorManager::ModelRing functions.
// ----------------------------------------------------------------------------

Color ColorManager::ModelRing::GetColorForPoint(const Point2f &point) {
    using ion::math::ArcTangent2;
    using ion::math::Length;

    // The hue is the angle around the origin.
    float hue = -ArcTangent2(point[1], point[0]).Degrees() / 360.0f;
    if (hue < 0)
        hue += 1;

    // The radius determines the saturation and value.
    const float radius = Length(Vector2f(point));
    const float t = Clamp((radius - kRingInnerRadius_) /
                          (kRingOuterRadius_ - kRingInnerRadius_), 0, 1);
    const float sat = Lerp(t, kMinModelSaturation_, kMaxModelSaturation_);
    const float val = Lerp(t, kMinModelValue_,      kMaxModelValue_);

    return Color::FromHSV(hue, sat, val);
}

Point2f ColorManager::ModelRing::GetPointForColor(const Color &color) {
    return Point2f(0, 0); // XXXX
}

// ----------------------------------------------------------------------------
// ColorManager functions.
// ----------------------------------------------------------------------------

std::unordered_map<std::string, Color> ColorManager::special_map_;


ColorManager::ColorManager() {
    // Create a random number generator with a constant seed for repeatability.
    std::default_random_engine            gen(0x12345678);
    std::uniform_real_distribution<float> dist(0, 1);

    const int kModelColorCount = 12;

    const float min_sat = GetMinModelSaturation();
    const float max_sat = GetMaxModelSaturation();
    const float min_val = GetMinModelValue();
    const float max_val = GetMaxModelValue();

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
        const float s = Lerp(dist(gen), min_sat, max_sat);
        const float v = Lerp(dist(gen), min_val, max_val);
        model_colors_.push_back(Color::FromHSV(h, s, v));
    }
}

// Constant functions.
float ColorManager::GetMinModelSaturation() { return .25f; }
float ColorManager::GetMaxModelSaturation() { return .40f; }
float ColorManager::GetMinModelValue()      { return .90f; }
float ColorManager::GetMaxModelValue()      { return .95f; }

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
    ASSERT(! Util::MapContains(special_map_, name));
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
