#pragma once

#include <memory>
#include <vector>

#include "Math/Types.h"

/// ColorManager is responsible for managing colors for Models.
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

  private:
    /// Predefined pseudo-random colors for Models.
    std::vector<Color> model_colors_;

    /// Next index to use for a Model color.
    size_t next_model_index_ = 0;
};

typedef std::shared_ptr<ColorManager> ColorManagerPtr;
