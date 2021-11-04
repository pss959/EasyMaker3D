#pragma once

/// The Action enum represents different actions that are supported by the
/// application.
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

    // General tools. Note that the order here defines the order on the shelf.
    kColorTool,
    kComplexityTool,
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
    kInspectSelection,
    kToggleBuildVolume,
    kToggleShowEdges,

    // Visibility.
    kHideSelected,
    kShowAll,

    // Radial menus.
    kToggleLeftRadialMenu,
    kToggleRightRadialMenu,

    // Other.
    kEditName,

#if defined DEBUG
    // Debugging.
    kPrintBounds,
    kPrintMatrices,
    kPrintNodesAndShapes,
    kPrintPanes,
    kPrintScene,
    kReloadScene,
#endif
};
