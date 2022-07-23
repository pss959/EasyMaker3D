#pragma once

#include "Math/Types.h"

/// The Defaults namespace has constants defining various default values used
/// in the application.
///
/// \ingroup Base
namespace Defaults {

/// Set to true to run without VR even if a headset is connected.
extern const bool     kIgnoreVR;

/// Height for the AppBoard.
extern const float    kAppBoardHeight;

/// Height for the ToolBoard.
extern const float    kToolBoardHeight;

/// Z offset for the KeyBoard from the other Boards.
extern const float    kKeyBoardZOffset;

/// Distance along the view direction from the camera of the plane containing
/// tooltips.
extern const float    kTooltipDistance;

/// Margin in normalized coordinates (0-1) to keep tooltips from being too
/// close to frustum edges.
extern const float    kTooltipMargin;

/// Fraction of maximum X or Z build volume size to use for the Stage radius.
extern const float    kStageRadiusFraction;

/// Maximum amount to scale the stage per second when restting.
extern const float    kMaxStageScaleChangePerSecond;

/// Maximum angle (in degrees) to change the stage rotation per second when
/// restting.
extern const float    kMaxStageAngleChangePerSecond;

/// Size to use for a build volume if it is not set by the user.
extern const Vector3f kBuildVolumeSize;

/// Base size to use for all 3D icons that appear on shelves. All icons are
/// scaled to fit this size in all 3 dimensions.
extern const float    kShelfIconSize;

/// Scale factor to apply to RadialMenu instances to attach to Controllers.
extern const float    kControllerRadialMenuScale;

/// Offset to apply to RadialMenu instances to attach to Controllers.
extern const Vector3f kControllerRadialMenuOffset;

/// Maximum angle between the controller guide direction and a specific
/// direction vector for grip hovering to be active.
extern const Anglef   kMaxGripHoverDirAngle;

/// How close points need to be to be snapped to (in stage coordinates).
extern const float    kSnapPointTolerance;

/// How close lengths need to be to be snapped to (in stage coordinates).
extern const float    kSnapLengthTolerance;

/// How close directions need to be to be snapped to (in degrees).
extern const float    kSnapDirectionTolerance;

///@}

/// Name of the default font for 3D text.
extern const char *   kFontName;

/// Default scale to apply to controller motion for grip drags.
extern const float    kGripDragScale;

/// Color to use for Models with invalid meshes.
extern const Color    kInvalidMeshColor;

/// Default maximum angle for applying a bevel Profile to an edge.
extern const float    kMaxBevelAngle;

/// Default complexity for new Models.
extern const float    kModelComplexity;

/// Conversion factor from Panel to world coordinates. Panel coordinates are
/// defined to make text readable at a reasonable "point size".
extern const float    kPanelToWorld;

/// Default length of delay (in seconds) before a tooltip is shown.
extern const float    kTooltipDelay;

}  // namespace Defaults
