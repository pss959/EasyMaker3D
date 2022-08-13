#pragma once

/// The Action enum represents different actions that are supported by the
/// application.
///
/// \ingroup Enums
enum class Action {
    // This is used in menus to indicate that no action is bound.
    kNone,

    // General operations.
    kUndo,
    kRedo,
    kQuit,

    // File operations.
    kOpenSessionPanel,
    kOpenSettingsPanel,
    kOpenInfoPanel,
    kOpenHelpPanel,

    // Model creation.
    kCreateBox,
    kCreateCylinder,
    kCreateImportedModel,
    kCreateRevSurf,
    kCreateSphere,
    kCreateText,
    kCreateTorus,

    // Model conversion.
    kConvertBevel,
    kConvertClip,
    kConvertMirror,

    // Model combining.
    kCombineCSGDifference,
    kCombineCSGIntersection,
    kCombineCSGUnion,
    kCombineHull,

    // General tools.
    kColorTool,
    kComplexityTool,
    kNameTool,
    kRotationTool,
    kScaleTool,
    kTranslationTool,

    // General tool switching.
    kSwitchToPreviousTool,
    kSwitchToNextTool,

    // Switching to specialized tool.
    kToggleSpecializedTool,

    // Complexity changes.
    kDecreaseComplexity,
    kIncreaseComplexity,

    // Precision changes.
    kDecreasePrecision,
    kIncreasePrecision,

    // Transform changes.
    kMoveToOrigin,

    // Selection operations.
    kSelectAll,
    kSelectNone,
    kSelectParent,
    kSelectFirstChild,
    kSelectPreviousSibling,
    kSelectNextSibling,

    // Clipboard operations.
    kDelete,
    kCut,
    kCopy,
    kPaste,
    kPasteInto,

    // Target toggles.
    kTogglePointTarget,
    kToggleEdgeTarget,

    // Layout/alignment actions.
    kLinearLayout,
    kRadialLayout,
    kToggleAxisAligned,

    // Model ordering operations.
    kMovePrevious,
    kMoveNext,

    // Viewing operations.
    kToggleInspector,
    kToggleBuildVolume,
    kToggleShowEdges,

    // Visibility.
    kHideSelected,
    kShowAll,

    // Radial menus.
    kToggleLeftRadialMenu,
    kToggleRightRadialMenu,

#if ! RELEASE_BUILD  // Actions defined to help with debugging.
    kReloadScene,
#endif
};

/// Convenience that returns true if the Action represents a toggle.
inline bool IsToggleAction(Action action) {
    switch (action) {
      case Action::kToggleSpecializedTool:
      case Action::kTogglePointTarget:
      case Action::kToggleEdgeTarget:
      case Action::kToggleAxisAligned:
      case Action::kToggleInspector:
      case Action::kToggleBuildVolume:
      case Action::kToggleShowEdges:
      case Action::kToggleLeftRadialMenu:
      case Action::kToggleRightRadialMenu:
        return true;
      default:
        return false;
    }
}
