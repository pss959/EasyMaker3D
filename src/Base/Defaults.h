#pragma once

#include "Math/Types.h"

/// The Defaults namespace has constants defining various default values used
/// in the application.
///
/// \ingroup Base
namespace Defaults {

/// Default size to use for a build volume if it is not set by the user.
extern const Vector3f kBuildVolumeSize;

/// Name of the default font for 3D text.
extern const char *   kFontName;

/// Base size to use for all 3D icons that appear on shelves. All icons are
/// scaled to fit this size in all 3 dimensions.
extern const float    kIconSize;

/// Color to use for Models with invalid meshes.
extern const Color    kInvalidMeshColor;

/// Default maximum angle for applying a bevel Profile to an edge.
extern const float    kMaxBevelAngle;

/// Default complexity for new Models.
extern const float    kModelComplexity;

/// Conversion factor from Panel to world coordinates. Panel coordinates are
/// defined to make text readable at a reasonable "point size".
extern const float    kPanelToWorld;

/// How close points need to be to be snapped to (in stage coordinates).
extern const float    kSnapPointTolerance;

/// How close lengths need to be to be snapped to (in stage coordinates).
extern const float    kSnapLengthTolerance;

/// How close directions need to be to be snapped to (in degrees).
extern const float    kSnapDirectionTolerance;

/// Default length of delay (in seconds) before a tooltip is shown.
extern const float    kTooltipDelay;

/// Default distance from the camera of the plane containing tooltips.
extern const float    kTooltipDistance;

}  // namespace Defaults
