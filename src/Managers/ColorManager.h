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
    /// The ColorTool uses a ring for selecting a Model color. This class
    /// represents that ring and allows points on it to be correlated with
    /// Model colors. This assumes the ring is centered on (0,0) with an outer
    /// radius of 1 and an inner radius of .45.
    class ModelRing {
      public:
        /// Returns the Color associated with the given point. The hue is
        /// determined by the angle of the point around the center. The
        /// saturation and value are determined by the distance from the
        /// center, clamped to the inner and outer radii.
        static Color   GetColorForPoint(const Point2f &point);

        /// Returns the point on the ring corresponding to the given Color. The
        /// saturation and value of the Color are clamped if necessary to
        /// ensure that the resulting point is on the ring.
        static Point2f GetPointForColor(const Color &color);
    };

    ColorManager();

    /// Resets the manager to its initial state.
    void Reset();

    /// \name Model colors.
    /// The colors used for Models in the application are limited to a range
    /// bounded by saturation and value quantities.
    ///@{

    /// Returns the minimum HSV saturation for Model colors.
    static float GetMinModelSaturation();

    /// Returns the maximum HSV saturation for Model colors.
    static float GetMaxModelSaturation();

    /// Returns the minimum HSV value for Model colors.
    static float GetMinModelValue();

    /// Returns the maximum HSV value for Model colors.
    static float GetMaxModelValue();

    /// Returns a pseudo-random color for a Model within the acceptable range
    /// of saturation and value.
    const Color & GetNextModelColor();

    ///@}

    /// Returns a color used to represent the given dimension.
    static Color GetColorForDimension(int dim);

    /// \name Special colors.
    /// The ColorManager maintains a set of named special colors.
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

typedef std::shared_ptr<ColorManager> ColorManagerPtr;
