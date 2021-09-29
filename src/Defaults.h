#pragma once

#include <string>

#include "Math/Types.h"

/// The Defaults namespace has constants defining various default values used
/// in the application.
namespace Defaults {

/// Default size to use for a build volume if it is not set by the user.
inline const Vector3f     kBuildVolumeSize{22, 30, 22};

/// Name of the default font family for 3D text. Should exist on all platforms.
inline const char *       kFontFamily = "Arial";

/// Name of the default font for 3D text. Should exist on all platforms.
inline const char *       kFontName = "Arial Regular";

/// Default maximum angle for applying a bevel Profile to an edge.
inline const float        kMaxBevelAngle = 120;

/// Default complexity for new Models.
inline const float        kModelComplexity = 0.3f;

/// Default length of delay (in seconds) before a tooltip is shown.
inline const float        kTooltipDelay = 1;

/// Returns the color to use for Models with invalid meshes.
inline const Color        kInvalidMeshColor{1, 0.16f, 0.18f, 1};

}  // namespace Defaults
