#pragma once

#include "Math/Types.h"

/// The Defaults namespace has constants defining various default values used
/// in the application.
namespace Defaults {

/// Default size to use for a build volume if it is not set by the user.
extern const Vector3f kBuildVolumeSize;

/// Name of the default font family for 3D text. Should exist on all platforms.
extern const char *   kFontFamily;

/// Name of the default font for 3D text. Should exist on all platforms.
extern const char *   kFontName;

/// Base size to use for all 3D icons that appear on shelves. All icons are
/// scaled to fit this size in all 3 dimensions.
extern const float    kIconSize;

/// Default maximum angle for applying a bevel Profile to an edge.
extern const float    kMaxBevelAngle;

/// Default complexity for new Models.
extern const float    kModelComplexity;

/// Default length of delay (in seconds) before a tooltip is shown.
extern const float    kTooltipDelay;

/// Returns the color to use for Models with invalid meshes.
extern const Color    kInvalidMeshColor;

}  // namespace Defaults
