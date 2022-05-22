#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Base/Memory.h"
#include "Math/Types.h"

DECL_SHARED_PTR(ColorMap);

/// ColorMap is responsible for mapping color names to colors and for choosing
/// pseudo-random colors.
///
/// \ingroup Base
class ColorMap {
  public:
    ColorMap();

    /// Resets the map to its initial state.
    void Reset();

    /// Returns a pseudo-random color for a Model within the acceptable range
    /// of saturation and value.
    const Color & GetNextModelColor();

    /// Returns a color used to represent the given dimension.
    static Color GetColorForDimension(int dim);

    /// \name Special colors.
    /// The ColorMap maintains a set of named special colors.
    ///@{

    /// Adds a special color corresponding to the given name.
    static void AddSpecialColor(const std::string &name, const Color &color);

    /// Returns the special color corresponding to the given name.
    static Color GetSpecialColor(const std::string &name);

    /// Clears all special colors.
    static void ClearSpecialColors();

    ///@}

 private:
    /// Maps special color name to Color.
    static std::unordered_map<std::string, Color> special_map_;

    /// Predefined pseudo-random colors for Models.
    std::vector<Color> model_colors_;

    /// Next index to use for a Model color.
    size_t next_model_index_ = 0;
};
