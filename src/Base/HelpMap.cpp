#include "Base/HelpMap.h"

#include "Util/Enum.h"


HelpMap::HelpMap() {
    const size_t action_count = Util::EnumCount<Action>();
    help_strings_.resize(action_count);

    auto set_hs = [&](Action action, const std::string &str){
        help_strings_[Util::EnumInt(action)] = str;
    };

    set_hs(Action::kQuit, "Exit the application");
    set_hs(Action::kUndo, "Undo the last command");
    set_hs(Action::kRedo, "Redo the last undone command");

    set_hs(Action::kOpenSessionPanel,
           "Open the panel to save or open session files");
    set_hs(Action::kOpenSettingsPanel, "Edit application settings");
    set_hs(Action::kOpenInfoPanel,
           "Open the panel to show information about selected models");
    set_hs(Action::kOpenHelpPanel, "Open the panel to access help");

    set_hs(Action::kCreateBox,           "Create a primitive Box model");
    set_hs(Action::kCreateCylinder,      "Create a primitive Cylinder model");
    set_hs(Action::kCreateImportedModel, "Import a model from a file");
    set_hs(Action::kCreateRevSurf,
           "Create a model that is a surface of revolution");
    set_hs(Action::kCreateSphere,        "Create a primitive Sphere model");
    set_hs(Action::kCreateText,          "Create a 3D Text model");
    set_hs(Action::kCreateTorus,         "Create a primitive Torus model");

    set_hs(Action::kConvertBevel, "Convert selected models to beveled models");
    set_hs(Action::kConvertClip,  "Convert selected models to clipped models");
    set_hs(Action::kConvertMirror,
           "Convert selected models to mirrored models");

    set_hs(Action::kCombineCSGDifference,
           "Create a CSG Difference from selected objects");
    set_hs(Action::kCombineCSGIntersection,
           "Create a CSG Intersection from selected objects");
    set_hs(Action::kCombineCSGUnion,
           "Create a CSG Union from selected objects");
    set_hs(Action::kCombineHull,
           "Create a model that is the convex hull of selected models");

    set_hs(Action::kColorTool,
           "Edit the color of the selected models");
    set_hs(Action::kComplexityTool,
           "Edit the complexity of the selected models");
    set_hs(Action::kNameTool,
           "Edit the name of the selected model");
    set_hs(Action::kRotationTool,
           "Rotate the selected models (Alt for in-place)");
    set_hs(Action::kScaleTool,
           "Change the size of the selected models (Alt for symmetric)");
    set_hs(Action::kTranslationTool,
           "Change the position of the selected models");

    set_hs(Action::kSwitchToPreviousTool,
           "Switch to the previous general tool");
    set_hs(Action::kSwitchToNextTool,
           "Switch to the next general tool");

    set_hs(Action::kToggleSpecializedTool,
           "TOGGLE: Switch between the current general tool and the"
           " specialized tool for the selected models");

    set_hs(Action::kDecreaseComplexity,
           "Decrease the complexity of the selected models by .05");
    set_hs(Action::kIncreaseComplexity,
           "Increase the complexity of the selected models by .05");

    set_hs(Action::kDecreasePrecision, "Decrease the current precision");
    set_hs(Action::kIncreasePrecision, "Increase the current precision");

    set_hs(Action::kMoveToOrigin, "Move the primary selection to the origin");

    set_hs(Action::kSelectAll,    "Select all top-level models");
    set_hs(Action::kSelectNone,   "Deselect all selected models");
    set_hs(Action::kSelectParent, "Select the parent of the primary selection");
    set_hs(Action::kSelectFirstChild,
           "Select the first child of the primary selection");
    set_hs(Action::kSelectPreviousSibling,
           "Select the previous sibling of the primary selection");
    set_hs(Action::kSelectNextSibling,
           "Select the next sibling of the primary selection");

    set_hs(Action::kDelete, "Delete all selected models");
    set_hs(Action::kCut,    "Cut all selected models to the clipboard");
    set_hs(Action::kCopy,   "Copy all selected models to the clipboard");
    set_hs(Action::kPaste,  "Paste all models from the clipboard");
    set_hs(Action::kPasteInto,
           "Paste all models from the clipboard as children of"
           " the selected model");

    set_hs(Action::kTogglePointTarget,
           "TOGGLE: Activate or deactivate the point target");
    set_hs(Action::kToggleEdgeTarget,
           "TOGGLE: Activate or deactivate the edge target");

    set_hs(Action::kLinearLayout,
           "Lay out the centers of the selected models along a\n"
           "line using the edge target");
    set_hs(Action::kRadialLayout,
           "Lay out selected models along a circular arc");

    set_hs(Action::kToggleAxisAligned,
           "TOGGLE: Transform models in local or global coordinates");

    set_hs(Action::kMovePrevious, "Move the selected model up in the order");
    set_hs(Action::kMoveNext,     "Move the selected model down in the order");

    set_hs(Action::kToggleInspector,
           "TOGGLE: Open or close the Inspector for the"
           " current primary selection");
    set_hs(Action::kToggleBuildVolume,
           "TOGGLE: Show or hide the translucent build volume");
    set_hs(Action::kToggleShowEdges,
           "TOGGLE: Show or hide edges on all models");

    set_hs(Action::kHideSelected, "Hide selected top-level models");
    set_hs(Action::kShowAll,      "Show all hidden top-level models");

    set_hs(Action::kToggleLeftRadialMenu,
           "TOGGLE: Show or hide the left radial menu");
    set_hs(Action::kToggleRightRadialMenu,
           "TOGGLE: Show or hide the right radial menu");
}

const std::string & HelpMap::GetHelpString(Action action) const {
    return help_strings_[Util::EnumInt(action)];
}
