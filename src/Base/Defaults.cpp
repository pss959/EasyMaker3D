#include "Base/Defaults.h"

namespace Defaults {

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
