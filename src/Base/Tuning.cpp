#include "Base/Tuning.h"

namespace TK {

// Application Settings
const char *   kApplicationName                  = "IMakerVR";
const char *   kVersionString                    = "0.2.1";

// Boards and Panels
const float    kAppBoardHeight                   = 14;
const float    kBoardTouchDistance               = .6f;
const float    kBoardTouchScale                  = .4f;
const float    kFloatingBoardYOffset             = 4;
const float    kMinBoardCanvasSize               = 4;
const float    kPaneZOffset                      = .02f;
const float    kPanelToWorldScale                = .06f;
const float    kTextInputPaneCursorWidth         = 40;
const float    kToolBoardHeight                  = 14;
const float    kToolBoardZOffset                 = 1;
const float    kTreePanelRectSelectZOffset       = .5f;

// Feedback and Haptics
const float    kAngularFeedbackArcRadius         = 10;
const float    kAngularFeedbackDegreesPerSegment = 4;
const float    kAngularFeedbackLineLength        = 24;
const float    kLinearFeedbackExtraHeight        = .5f;
const float    kLinearFeedbackExtraUprightLength = 2.f;
const float    kLinearFeedbackHeightScale        = 1.1f;
const float    kLinearFeedbackMinTextY           = 1.f;
const float    kLinearFeedbackTextYOffset        = 3.f;
const float    kVibrationAmplitude               = .3f;
const float    kVibrationFrequency               = 20;

// Fonts
const char *   k3DFont                           = "Arial-Regular";
const char *   kInfoPanelErrorFont               = "Verdana-Italic";
const char *   kInfoPanelHeaderFont              = "Verdana-Regular";
const char *   kInfoPanelRegularFont             = "DejaVu Sans Mono-Book";

// Interaction
const float    kColorToolGripDragScale           = 10;
const float    kGripDragScale                    = 80;
const double   kMaxEventHandlingTime             = .05;  // 20 FPS at worst.
const float    kPaneGripDragScale                = 1;
const float    kPinchHoverScale                  = .8f;
const float    kScrollingPaneKeyScrollAmount     = .5f;
const float    kScrollingPaneSpeed               = .4f;
const float    kScrollingPaneWheelScrollAmount   = .1f;

// Model Colors
const float    kColorRingInnerRadius             = .45f;
const float    kColorRingOuterRadius             = 1;
const int      kModelColorCount                  = 20;
const int      kModelHueRangeCount               = 8;
const float    kModelMaxSaturation               = .50f;
const float    kModelMaxValue                    = .95f;
const float    kModelMinSaturation               = .25f;
const float    kModelMinValue                    = .90f;

// Model Complexity
const int      kMaxCylinderSides                 = 120;
const int      kMaxRevSurfSides                  = 120;
const int      kMaxSphereRings                   = 60;
const int      kMaxSphereSectors                 = 60;
const float    kModelComplexity                  = 0.3f;

// Model and Other Sizes
const Vector3f kBuildVolumeSize                  = Vector3f{22, 30, 22};
const float    kDiscWidgetMaxScale               = 1000;
const float    kDiscWidgetMinScale               = .01f;
const float    kExtrudedTextLength               = 2;
const float    kFlatThickness                    = .001f;
const float    kInitialModelScale                = 4;
const float    kMaxCylinderRadius                = 30;
const float    kMaxTorusInnerRadius              = 10;
const float    kMaxTorusOuterRadius              = 20;
const float    kMinCylinderRadius                = .01;
const float    kMinTorusHoleRadius               = .01f;
const float    kMinTorusInnerRadius              = .01f;
const float    kScaleWidgetMaxLimit              = 100;
const float    kScaleWidgetMinLimit              = .01f;
const float    kTextHeight                       = 4;
const float    kTorusInnerRadius                 = .2f;

// Inspector
const float    kInspectorMousePitchAngle         = 120;
const float    kInspectorMouseYawAngle           = 180;
const float    kInspectorNonVRMaxScale           = 8;
const float    kInspectorNonVRMinScale           = .2f;
const float    kInspectorNonVRModelSize          = 14;
const float    kInspectorNonVRScaleMult          = .06f;
const float    kInspectorVRFraction              = .8f;
const float    kInspectorVRXOffset               = .14f;

// Layout
const float    kRLWArcAngleTextYOffset           = 1.6f;
const float    kRLWArcDegreesPerSegment          = 4;
const float    kRLWArcLineWidth                  = .2f;
const float    kRLWArcRadiusScale                = .6f;   // Relative to radius.
const float    kRLWMinRadiusForSpokes            = 1.5f;
const float    kRLWRadiusTextYOffset             = 1;
const float    kRLWRingMaxInnerRadius            = .3f;
const int      kRLWRingMaxSectorCount            = 72;
const float    kRLWRingMinInnerRadius            = .2f;
const int      kRLWRingMinSectorCount            = 36;
const int      kRLWRingRingCount                 =  8;
const float    kRLWSpokeScale                    = 1.1f;  // Relative to radius.
const float    kRLWStartAngleTextYOffset         = 1;

// Lighting and Rendering
const int      kColorRingImageSize               = 256;
const int      kDepthMapSize                     = 2048;
const float    kShadowPassLightFar               = 202;
const float    kShadowPassLightHalfSize          = 80;
const float    kShadowPassLightNear              = -20;
const int      kStageImageSize                   = 1024;
const int      kVRSampleCount                    = 4;

// Radial Menus
const Vector3f kControllerRadialMenuOffset       = Vector3f(0, .06f, -.1f);
const float    kControllerRadialMenuScale        = .5f;
const int      kRadialMenuCirclePointCount       = 72;
const float    kRadialMenuInnerAngleMargin       = 8;
const float    kRadialMenuInnerRadius            = 1;
const float    kRadialMenuOuterRadius            = 4;
const float    kRadialMenuRadiusMargin           = .25f;

// Shelves and 3D Icons
const float    kShelfIconDistanceScale           = .015f;
const float    kShelfIconMargin                  = 1;
const float    kShelfIconSize                    = 3.6f;
const float    kShelfIconYOffset                 = 1.2f;

// Stage
const int      kStageOriginRadius                = 5;
const float    kStageRadiusFraction              = .8f;

// Text
const char *   kDegreeSign                       = "°";
const float    kInfoPanelIndent                  = .04f;
const int      kInfoPanelLabelWidth              = 16;
const int      kParserWriterIndent               = 2;

// Time and Animation
const float    kClickTimeout                     = .25f;
const float    kLongPressTime                    = .6f;
const float    kMaxHeightChangePerSecond         = .4f;
const float    kMaxStageAngleChangePerSecond     = 90;
const float    kMaxStageScaleChangePerSecond     = 4;
const float    kMaxViewAngleChangePerSecond      = 90.f;
const float    kModelAnimationDuration           = 1.2f;
const float    kModelAnimationHeight             = 80;

// Tolerances and Precision
const float    kMaxBevelAngle                    = 120;
const Anglef   kMaxDiscWidgetScaleAngle          = Anglef::FromDegrees(10);
const Anglef   kMaxGripHoverDirAngle             = Anglef::FromDegrees(20);
const float    kMeshCleanTolerance               = 1.e-5f;
const float    kMinClippedSize                   = .01f;
const float    kMinDiscWidgetMotion              = .01f;
const float    kMinGripControllerDistance        = .04f;
const Anglef   kMinGripOrientationAngleChange    = Anglef::FromDegrees(10);
const Anglef   kMinRayAngleChange                = Anglef::FromDegrees(1);
const float    kMinTouchControllerDistance       = .025f;
const float    kMinTouchZMotion                  = .02f;
const float    kProfilePanePointTolerance        = .04f;
const float    kSTLPrecision                     = .0001f;
const float    kSnapDirectionTolerance           = 15;
const float    kSnapLengthTolerance              = 1;
const float    kSnapPointTolerance               = 1;

// Tools
const float    kClipToolArrowScale               = 1.6f;  // Must be > sqrt(2).
const float    kClipToolPlaneScale               = 1.5f;
const float    kClipToolRotatorScale             = 1.1f;
const float    kColorToolMarkerRadius            = .06f;
const float    kMirrorToolPlaneOpacity           = .2f;
const float    kMirrorToolPlaneScale             = 1.8f;
const float    kRotationToolRadiusScale          = .51f;
const float    kRotationToolSphereRadiusScale    = .9f;
const float    kTranslationToolExtraStickLength  = .4f;

// Tooltips
const float    kTooltipDelay                     = 1;
const float    kTooltipDistance                  = 2;
const float    kTooltipMargin                    = .05f;

// Viewing and Windows
const float    kFOVHalfAngle                     = 30;
const float    kFarDistance                      = 20;
const float    kNearDistance                     = .01f;
const float    kVRFarDistance                    = 300;
const float    kVRNearDistance                   = .1f;
const float    kWindowAspectRatio                = 16.f / 9.f;  // HD ratio.
const int      kWindowHeight                     = 600;

}  // namespace TK
