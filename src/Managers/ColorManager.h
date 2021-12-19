#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Math/Types.h"

/// ColorManager is responsible for managing colors for Models and other
/// important objects in the scene.
///
// \ingroup Managers
class ColorManager {
  public:
    ColorManager();

    /// Resets the manager to its initial state.
    void Reset();

    /// Returns a pseudo-random color for a Model within the acceptable range
    /// of saturation and value.
    const Color & GetNextModelColor();

    /// Returns a color used to represent the given dimension.
    static Color GetColorForDimension(int dim);

    /// Adds a special color corresponding to the given name.
    static void AddSpecialColor(const std::string &name, const Color &color);

    /// Returns the special color corresponding to the given name.
    static Color GetSpecialColor(const std::string &name);

    /// Clears all special colors.
    static void ClearSpecialColors();

 private:
    /// Maps special color name to Color.
    static std::unordered_map<std::string, Color> special_map_;

    /// Predefined pseudo-random colors for Models.
    std::vector<Color> model_colors_;

    /// Next index to use for a Model color.
    size_t next_model_index_ = 0;
};

typedef std::shared_ptr<ColorManager> ColorManagerPtr;
