#pragma once

#include "Math/Types.h"

/// The Defaults namespace has constants defining various default values used
/// in the application. The values are set in Defaults.cpp so that tweaking
/// values has minimal effect on compilation time.
///
/// \ingroup Base
namespace Defaults {

/// Time in seconds to wait for multiple button clicks.
extern const float    kClickTimeout;

/// Minimum time in seconds for a button press to be considered a long press.
extern const float    kLongPressTime;

/// Angle (in degrees) the Inspector should rotate the Model horizontally for a
/// mouse drag across the entire window width.
extern const float    kInspectorMouseYawAngle;

/// Angle (in degrees) the Inspector should rotate the Model vertically for a
/// mouse drag across the entire window height.
extern const float    kInspectorMousePitchAngle;

/// Extra height added to the crossbar used for LinearFeedback.
extern const float    kLinearFeedbackExtraHeight;

/// Extra length added to the uprights used for LinearFeedback.
extern const float    kLinearFeedbackExtraUprightLength;

/// Amount to scale scene bounds so that the crossbar used for LinearFeedback
/// does not intersect anything.
extern const float    kLinearFeedbackHeightScale;

/// Y Offset above the crossbar for the text displayed for LinearFeedback.
extern const float    kLinearFeedbackTextYOffset;

/// Minimum Y value for text display for LinearFeedback.
extern const float    kLinearFeedbackMinTextY;

/// Length of the lines used for AngularFeedback.
extern const float    kAngularFeedbackLineLength;

/// Radius of the arc used for AngularFeedback.
extern const float    kAngularFeedbackArcRadius;

// Number of degrees per arc segment in AngularFeedback.
extern const float    kAngularFeedbackDegreesPerSegment;

/// Height at which Model placement animation starts.
extern const float    kModelAnimationHeight;

/// Duration (in seconds) for Model placement animation.
extern const float    kModelAnimationDuration;

/// Size in pixels of the square image used to show the color ring image for
/// the ColorTool.
extern const int      kColorRingImageSize;

/// Size in pixels of the square image used to show the Stage grid.
extern const int      kStageImageSize;

/// Radius in pixels of the circle on the Stage that shows the origin.
extern const int      kStageOriginRadius;

/// Height to use for the main application window.
extern const int      kWindowHeight;

/// Aspect ratio to use for the main application window.
extern const float    kWindowAspectRatio;

/// Number of samples to use for VR multisampling to render into framebuffer
/// objects.
extern const int      kVRSampleCount;

/// Near clipping distance for a VR view frustum.
extern const float    kVRNearDistance;

/// Far clipping distance for a VR view frustum.
extern const float    kVRFarDistance;

/// Frequency of controller vibration for haptic feedback.
extern const float    kVibrationFrequency;

/// Amplitude of controller vibration for haptic feedback.
extern const float    kVibrationAmplitude;

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

/// Maximum amount to scale the stage per second when resetting.
extern const float    kMaxStageScaleChangePerSecond;

/// Maximum angle (in degrees) to change the stage rotation per second when
/// resetting.
extern const float    kMaxStageAngleChangePerSecond;

// Maximum amount to change the height per second when resetting.
extern const float    kMaxHeightChangePerSecond;

// Maximum amount to change the view angle (for the height slider, in degrees)
// per second when resetting.
extern const float    kMaxViewAngleChangePerSecond;

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
