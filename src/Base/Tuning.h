#pragma once

#include "Math/Types.h"

/// \file
/// This file defines constants that can be modified to tune various aspects of
/// the application. It defines the \c K namespace for the constants for
/// brevity.
///
/// \ingroup Tuning

/// The TK (tuning constants) namespace has constants defining various values
/// that can be used to tune the application. The values are set in Tuning.cpp
/// so that modifying values to tune application behavior has minimal effect on
/// compilation time.
///
/// \ingroup Tuning
namespace TK {

/// \name Application Settings
///@{

/// Name of the application.
extern const char *   kApplicationName;

/// Application version as a string.
extern const char *   kVersionString;

///@}

/// \name Boards and Panels
///@{

/// Height for the AppBoard.
extern const float    kAppBoardHeight;

/// Distance from the VR camera to a Board to enable touch interaction.
extern const float    kBoardTouchDistance;

/// Scale appliied to a Board at touch distance based on FOV.
extern const float    kBoardTouchScale;

/// Amount to raise a floating Board above the stage to make it visible when
/// not in VR.
extern const float    kFloatingBoardYOffset;

/// Minimum size for either canvas dimension in a Board.
extern const float    kMinBoardCanvasSize;

/// Amount to offset Pane items in Z to avoid Z-fighting.
extern const float    kPaneZOffset;

/// Conversion factor from Panel to world coordinates. Panel coordinates are
/// defined to make text readable at a reasonable "point size".
extern const float    kPanelToWorldScale;

/// Width of the cursor in a TextInputPane in Pane coordinates.
extern const float    kTextInputPaneCursorWidth;

/// Height for the ToolBoard.
extern const float    kToolBoardHeight;

/// Amount to offset the ToolBoard in Z from the front of a Model when
/// attaching a PanelTool to it (and not in VR).
extern const float    kToolBoardZOffset;

/// Z offset of the rectangle selection geometry in the TreePanel.
extern const float    kTreePanelRectSelectZOffset;

///@}

/// \name Feedback and Haptics
///@{

/// Radius of the arc used for AngularFeedback.
extern const float    kAngularFeedbackArcRadius;

// Number of degrees per arc segment in AngularFeedback.
extern const float    kAngularFeedbackDegreesPerSegment;

/// Length of the lines used for AngularFeedback.
extern const float    kAngularFeedbackLineLength;

/// Extra height added to the crossbar used for LinearFeedback.
extern const float    kLinearFeedbackExtraHeight;

/// Extra length added to the uprights used for LinearFeedback.
extern const float    kLinearFeedbackExtraUprightLength;

/// Amount to scale scene bounds so that the crossbar used for LinearFeedback
/// does not intersect anything.
extern const float    kLinearFeedbackHeightScale;

/// Minimum Y value for text display for LinearFeedback.
extern const float    kLinearFeedbackMinTextY;

/// Y Offset above the crossbar for the text displayed for LinearFeedback.
extern const float    kLinearFeedbackTextYOffset;

/// Amplitude of controller vibration for haptic feedback.
extern const float    kVibrationAmplitude;

/// Frequency of controller vibration for haptic feedback.
extern const float    kVibrationFrequency;

///@}

/// \name Fonts
///@{

/// Font to use for 3D text.
extern const char *   k3DFont;

/// Font to use for error text in the InfoPanel.
extern const char *   kInfoPanelErrorFont;

/// Font to use for header text in the InfoPanel.
extern const char *   kInfoPanelHeaderFont;

/// Font to use for regular text in the InfoPanel.
extern const char *   kInfoPanelRegularFont;

///@}

/// \name Interaction
///@{

/// Amount to scale grip drags to move the ColorTool marker.
extern const float    kColorToolGripDragScale;

/// Scale to apply to controller motion for grip drags.
extern const float    kGripDragScale;

/// Scale to apply to controller motion for grip drags in Panes.
extern const float    kPaneGripDragScale;

/// Amount to scale the pinch hover sphere for a controller per unit distance.
extern const float    kPinchHoverScale;

/// Amount to scroll a ScrollingPane by up or down keys.
extern const float    kScrollingPaneKeyScrollAmount;

/// Scrolling speed in a ScrollingPane.
extern const float    kScrollingPaneSpeed;

/// Amount to scroll a ScrollingPane by a unit change in the scroll wheel.
extern const float    kScrollingPaneWheelScrollAmount;

///@}

/// \name Model Colors
///@{

/// Inner radius of the ColorTool's ColorRing.
extern const float    kColorRingInnerRadius;

/// Outer radius of the ColorTool's ColorRing.
extern const float    kColorRingOuterRadius;

/// Number of distinct randomly generated Model colors.
extern const int      kModelColorCount;

/// Number of hue ranges used for generating Model colors.
extern const int      kModelHueRangeCount;

/// Maximum HSV Saturation range for Model colors.
extern const float    kModelMaxSaturation;

/// Maximum HSV Value range for Model colors.
extern const float    kModelMaxValue;

/// Minimum HSV Saturation range for Model colors.
extern const float    kModelMinSaturation;

/// Minimum HSV Value range for Model colors.
extern const float    kModelMinValue;

///@}

/// \name Model Complexity
///@{

/// Number of sides to use for a CylinderModel with complexity 1.
extern const int      kMaxCylinderSides;

/// Number of sides to use for a RevSurfModel with complexity 1.
extern const int      kMaxRevSurfSides;

/// Number of rings to use for a SphereModel with complexity 1.
extern const int      kMaxSphereRings;

/// Number of sectors to use for a SphereModel with complexity 1.
extern const int      kMaxSphereSectors;

/// Initial complexity for new Models.
extern const float    kModelComplexity;

///@}

/// \name Model and Other Sizes
///@{

/// Size to use for a build volume if it is not set by the user.
extern const Vector3f kBuildVolumeSize;

/// Maximum allowable scale factor produced by a DiscWidget.
extern const float    kDiscWidgetMaxScale;

/// Minimum allowable scale factor produced by a DiscWidget.
extern const float    kDiscWidgetMinScale;

/// Extruded length of a TextModel.
extern const float    kExtrudedTextLength;

/// Size to use for "flat" dimension of bounds of 2D shapes.
extern const float    kFlatThickness;

/// Amount to scale a Model when it is first created.
extern const float    kInitialModelScale;

/// Maximum allowable radius for a CylinderModel (in the CylinderTool).
extern const float    kMaxCylinderRadius;

/// Maximum inner radius of a TorusModel (for the TorusTool).
extern const float    kMaxTorusInnerRadius;

/// Maximum outer radius of a TorusModel (for the TorusTool).
extern const float    kMaxTorusOuterRadius;

/// Minimum allowable radius for a CylinderModel.
extern const float    kMinCylinderRadius;

/// Minimum radius of the hole of a TorusModel.
extern const float    kMinTorusHoleRadius;

/// Minimum inner radius of a TorusModel.
extern const float    kMinTorusInnerRadius;

/// Maximum allowable scale factor produced by a ScaleWidget.
extern const float    kScaleWidgetMaxLimit;

/// Minimum allowable scale factor produced by a ScaleWidget.
extern const float    kScaleWidgetMinLimit;

/// Height (in Z direction) of a line of text in a TextModel.
extern const float    kTextHeight;

/// Initial inner radius of a TorusModel.
extern const float    kTorusInnerRadius;

///@}

/// \name Inspector
///@{

/// Angle (in degrees) the Inspector should rotate the Model vertically for a
/// mouse drag across the entire window height.
extern const float    kInspectorMousePitchAngle;

/// Angle (in degrees) the Inspector should rotate the Model horizontally for a
/// mouse drag across the entire window width.
extern const float    kInspectorMouseYawAngle;

/// Maximum scale for a Model being inspected (not in VR).
extern const float    kInspectorNonVRMaxScale;

/// Minimum scale for a Model being inspected (not in VR).
extern const float    kInspectorNonVRMinScale;

/// Size to use for a Model being inspected (not in VR).
extern const float    kInspectorNonVRModelSize;

/// Amount to scale per unit change in scroll wheel for a Model being inspected
/// (not in VR).
extern const float    kInspectorNonVRScaleMult;

/// Fraction of the controller size to use for a Model inspected in VR.
extern const float    kInspectorVRFraction;

/// Offset in X from the controller for a Model inspected in VR.
extern const float    kInspectorVRXOffset;

///@}

/// \name Layout
///@{

/// Y offset of arc angle text feedback in the RadialLayoutWidget.
extern const float    kRLWArcAngleTextYOffset;

/// Number of degrees per arc segment in the RadialLayoutWidget.
extern const float    kRLWArcDegreesPerSegment;

/// Width of lines in the RadialLayoutWidget.
extern const float    kRLWArcLineWidth;

/// Scale (relative to the radius) of the arc in the RadialLayoutWidget.
extern const float    kRLWArcRadiusScale;

/// Minimum radius needed to show the spokes and arc in the RadialLayoutWidget.
extern const float    kRLWMinRadiusForSpokes;

/// Y offset of radius text feedback in the RadialLayoutWidget.
extern const float    kRLWRadiusTextYOffset;

/// Maximum inner radius for the RadialLayoutWidget ring.
extern const float    kRLWRingMaxInnerRadius;

/// Maximum number of sectors to use for the RadialLayoutWidget ring.
extern const int      kRLWRingMaxSectorCount;

/// Minimum inner radius for the RadialLayoutWidget ring.
extern const float    kRLWRingMinInnerRadius;

/// Minimum number of sectors to use for the RadialLayoutWidget ring.
extern const int      kRLWRingMinSectorCount;

/// Number of rings to use for the RadialLayoutWidget ring.
extern const int      kRLWRingRingCount;

/// Scale (relative to the radius) to apply to spokes in the RadialLayoutWidget.
extern const float    kRLWSpokeScale;

/// Y offset of start angle text feedback in the RadialLayoutWidget.
extern const float    kRLWStartAngleTextYOffset;

///@}

/// \name Lighting and Rendering
///@{

/// Size in pixels of the square image used to show the color ring image for
/// the ColorTool.
extern const int      kColorRingImageSize;

/// Size in pixels of the depth map used in a ShadowPass.
extern const int      kDepthMapSize;

/// Far distance used for orthographic frustum for lights in a ShadowPass.
extern const float    kShadowPassLightFar;

/// Half size used for orthographic frustum for lights in a ShadowPass.
extern const float    kShadowPassLightHalfSize;

/// Near distance used for orthographic frustum for lights in a ShadowPass.
extern const float    kShadowPassLightNear;

/// Size in pixels of the square image used to show the Stage grid.
extern const int      kStageImageSize;

/// Number of samples to use for VR multisampling to render into framebuffer
/// objects.
extern const int      kVRSampleCount;

///@}

/// \name Radial Menus
///@{

/// Offset to apply to RadialMenu instances to attach to Controllers.
extern const Vector3f kControllerRadialMenuOffset;

/// Scale factor to apply to RadialMenu instances to attach to Controllers.
extern const float    kControllerRadialMenuScale;

/// Number of points to use for each RadialMenu circle.
extern const int      kRadialMenuCirclePointCount;

/// Angle in degrees forming a margin on the inner circle between buttons of a
/// RadialMenu.
extern const float    kRadialMenuInnerAngleMargin;

/// Radius of the inner circle of a RadialMenu.
extern const float    kRadialMenuInnerRadius;

/// Radius of the outer circle of a RadialMenu.
extern const float    kRadialMenuOuterRadius;

/// Margin along the radius between circles and buttons in a RadialMenu.
extern const float    kRadialMenuRadiusMargin;

///@}

/// \name Shelves and 3D Icons
///@{

/// Multiplier for scaling shelf icons proportional to distance from the camera
/// to keep the sizes relatively constant on the screen.
extern const float    kShelfIconDistanceScale;

/// Margin around and between shelf icons.
extern const float    kShelfIconMargin;

/// Base size to use for all 3D icons that appear on shelves. All icons are
/// scaled to fit this size in all 3 dimensions.
extern const float    kShelfIconSize;

/// Amount to offset shelf icons in Y.
extern const float    kShelfIconYOffset;

///@}

/// \name Stage
///@{

/// Radius in pixels of the circle on the Stage that shows the origin.
extern const int      kStageOriginRadius;

/// Fraction of maximum X or Z build volume size to use for the Stage radius.
extern const float    kStageRadiusFraction;

///@}

/// \name Text
///@{

/// Degree sign used in angle feedback.
extern const char *   kDegreeSign;

/// Amount to indent lines of text in the InfoPanel.
extern const float    kInfoPanelIndent;

/// Number of characters to use for labels in the InfoPanel.
extern const int      kInfoPanelLabelWidth;

/// Number of spaces to indent per level for the Parser::Writer.
extern const int      kParserWriterIndent;

///@}

/// \name Time and Animation
///@{

/// Time in seconds to wait for multiple button clicks.
extern const float    kClickTimeout;

/// Minimum time in seconds for a button press to be considered a long press.
extern const float    kLongPressTime;

// Maximum amount to change the height per second when resetting.
extern const float    kMaxHeightChangePerSecond;

/// Maximum angle (in degrees) to change the stage rotation per second when
/// resetting.
extern const float    kMaxStageAngleChangePerSecond;

/// Maximum amount to scale the stage per second when resetting.
extern const float    kMaxStageScaleChangePerSecond;

// Maximum amount to change the view angle (for the height slider, in degrees)
// per second when resetting.
extern const float    kMaxViewAngleChangePerSecond;

/// Duration (in seconds) for Model placement animation.
extern const float    kModelAnimationDuration;

/// Height at which Model placement animation starts.
extern const float    kModelAnimationHeight;

///@}

/// \name Tolerances and Precision
///@{

/// Maximum angle for applying a bevel Profile to an edge.
extern const float    kMaxBevelAngle;

/// The angle between a DiscWidget motion vector and the direction to the
/// center of the widget has to be smaller than this for the motion to be
/// considered a scale.
extern const Anglef   kMaxDiscWidgetScaleAngle;

/// Maximum angle between the controller guide direction and a specific
/// direction vector for grip hovering to be active.
extern const Anglef   kMaxGripHoverDirAngle;

/// How close coordinates need to be to zero to be considered zero when
/// "cleaning" mesh points.
extern const float    kMeshCleanTolerance;

/// Minimum geometry size the ClipTool must leave along the clip direction.
extern const float    kMinClippedSize;

/// Minimum motion in the DiscWidget to choose scaling vs. rotation.
extern const float    kMinDiscWidgetMotion;

/// Minimum distance (in world-coordinates) for a controller to move to be
/// considered enough for a grip drag.
extern const float    kMinGripControllerDistance;

/// Minimum angle difference between two controller orientations to be
/// considered enough for a grip drag.
extern const Anglef   kMinGripOrientationAngleChange;

/// Minimum angle between two ray directions to be considered enough for a
/// pointer-based drag.
extern const Anglef   kMinRayAngleChange;

/// Minimum world-space distance for a controller to move in X or Y to be
/// considered a potential touch drag operation.
extern const float    kMinTouchControllerDistance;

/// Minimum amount the controller must move in Z to deactivate after a touch.
extern const float    kMinTouchZMotion;

/// Maximum distance to a ProfilePane point to be considered close to it.
extern const float    kProfilePanePointTolerance;

/// Precision to use for points when writing STL files.
extern const float    kSTLPrecision;

/// How close directions need to be to be snapped to (in degrees).
extern const float    kSnapDirectionTolerance;

/// How close lengths need to be to be snapped to (in stage coordinates).
extern const float    kSnapLengthTolerance;

/// How close points need to be to be snapped to (in stage coordinates).
extern const float    kSnapPointTolerance;

///@}

/// \name Tools
///@{

/// Scale factor for ClipTool arrow slider.
extern const float    kClipToolArrowScale;

/// Scale factor for clickable ClipTool plane.
extern const float    kClipToolPlaneScale;

/// Scale factor for ClipTool rotator sphere.
extern const float    kClipToolRotatorScale;

/// Radius of the marker circle in the ColorTool.
extern const float    kColorToolMarkerRadius;

/// Opacity of clickable planes in MirrorTool.
extern const float    kMirrorToolPlaneOpacity;

/// Scale of MirrorTool planes with respect to the attached Model.
extern const float    kMirrorToolPlaneScale;

/// Scale factor (relative to Model bounds diagonal length) for the radius of a
/// RotationTool.
extern const float    kRotationToolRadiusScale;

/// Scale factor for the RotationTool free rotation sphere relative to the axis
/// rotator radii.
extern const float    kRotationToolSphereRadiusScale;

/// Extra length to add to the scaler stick in a TranslationTool.
extern const float    kTranslationToolExtraStickLength;

///@}

/// \name Tooltips
///@{

/// Initial length of the delay (in seconds) before a tooltip is shown.
extern const float    kTooltipDelay;

/// Distance along the view direction from the camera of the plane containing
/// tooltips.
extern const float    kTooltipDistance;

/// Margin in normalized coordinates (0-1) to keep tooltips from being too
/// close to frustum edges.
extern const float    kTooltipMargin;

///@}

/// \name Viewing and Windows
///@{

/// Half-field-of-view angle (in degrees) for the default Frustum.
extern const float    kFOVHalfAngle;

/// Far clipping plane distance for the default frustum.
extern const float    kFarDistance;

/// Near clipping plane distance for the default frustum.
extern const float    kNearDistance;

/// Far clipping distance for a VR view frustum.
extern const float    kVRFarDistance;

/// Near clipping distance for a VR view frustum.
extern const float    kVRNearDistance;

/// Aspect ratio to use for the main application window.
extern const float    kWindowAspectRatio;

/// Height to use for the main application window.
extern const int      kWindowHeight;

///@}

}  // namespace TK
