#include "Base/Defaults.h"

namespace Defaults {

const float    kInspectorNonVRModelSize = 14;
const float    kInspectorNonVRScaleMult = .06f;
const float    kInspectorNonVRMinScale = .2f;
const float    kInspectorNonVRMaxScale = 8;
const float    kInspectorVRFraction = .8f;
const float    kInspectorVRXOffset = .14f;
const float    kPinchHoverScale = .8f;
const float    kBoardTouchDistance = .6f;
const float    kClickTimeout = .25f;
const float    kLongPressTime = .6f;
const float    kInspectorMouseYawAngle = 180;
const float    kInspectorMousePitchAngle = 120;
const float    kLinearFeedbackExtraHeight      = .5f;
const float    kLinearFeedbackExtraUprightLength  = 2.f;
const float    kLinearFeedbackHeightScale       = 1.1f;
const float    kLinearFeedbackTextYOffset = 3.f;
const float    kLinearFeedbackMinTextY         = 1.f;
const float    kAngularFeedbackLineLength = 24;
const float    kAngularFeedbackArcRadius = 10;
const float    kAngularFeedbackDegreesPerSegment = 4;
const float    kModelAnimationHeight = 80;
const float    kModelAnimationDuration = 1.2f;
const int      kColorRingImageSize = 256;
const int      kStageImageSize = 1024;
const int      kStageOriginRadius = 5;
const int      kWindowHeight = 600;
const float    kWindowAspectRatio = 16.f / 9.f;  // HD ratio.
const int      kVRSampleCount = 4;
const float    kVRNearDistance = .1f;
const float    kVRFarDistance = 300;
const float    kVibrationFrequency = 20;
const float    kVibrationAmplitude = .3f;
const bool     kIgnoreVR = false;
const float    kMaxStageScaleChangePerSecond = 4;
const float    kMaxStageAngleChangePerSecond = 90;
const float    kMaxHeightChangePerSecond = .4f;
const float    kMaxViewAngleChangePerSecond = 90.f;
const float    kStageRadiusFraction = .8f;
const float    kTooltipMargin = .05f;
const float    kControllerRadialMenuScale = .5f;
const Vector3f kControllerRadialMenuOffset(0, .06f, -.1f);
const Anglef   kMaxGripHoverDirAngle = Anglef::FromDegrees(20);
const float    kAppBoardHeight = 14;
const float    kToolBoardHeight = 14;
const float    kKeyBoardZOffset = .1f;
const Vector3f kBuildVolumeSize{22, 30, 22};
const char *   kFontName               = "Arial-Regular";
const float    kGripDragScale          = 80;
const float    kShelfIconSize          = 3.6f;
const Color    kInvalidMeshColor{1, 0.16f, 0.18f, 1};
const float    kMaxBevelAngle          = 120;
const float    kMinimumPaneTextHeight  = 4.f;
const float    kModelComplexity        = 0.3f;
const float    kPanelToWorld           = .06f;
const float    kSnapPointTolerance     = 1;
const float    kSnapLengthTolerance    = 1;
const float    kSnapDirectionTolerance = 15;
const float    kTooltipDelay           = 1;
const float    kTooltipDistance        = 2;

}  // namespace Defaults
