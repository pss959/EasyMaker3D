#include "Base/ActionMap.h"

#include "Util/Assert.h"
#include "Util/Enum.h"

ActionMap::ActionMap() :
    category_map_(BuildCategoryMap_()),
    action_map_(BuildActionMap_(category_map_)) {
}

ActionCategory ActionMap::GetCategoryForAction(Action action) const {
    const auto it = action_map_.find(action);
    ASSERT(it != action_map_.end());
    return it->second;
}

const std::vector<Action> & ActionMap::GetActionsInCategory(
    ActionCategory cat) const {
    const auto it = category_map_.find(cat);
    ASSERT(it != category_map_.end());
    return it->second;
}

ActionMap::CategoryMap_ ActionMap::BuildCategoryMap_() {
    CategoryMap_ map({
       { ActionCategory::kBasics,         // LCOV_EXCL_LINE [bug in gcovr]
          { Action::kCopy,
            Action::kCut,
            Action::kDelete,
            Action::kDuplicate,
            Action::kPaste,
            Action::kPasteInto,
            Action::kQuit,
            Action::kRedo,
            Action::kUndo }
        },
        { ActionCategory::kCombination,   // LCOV_EXCL_LINE [bug in gcovr]
          { Action::kCombineCSGDifference,
            Action::kCombineCSGIntersection,
            Action::kCombineCSGUnion,
            Action::kCombineHull }
        },
        { ActionCategory::kConversion,    // LCOV_EXCL_LINE [bug in gcovr]
          { Action::kConvertBend,
            Action::kConvertBevel,
            Action::kConvertClip,
            Action::kConvertMirror,
            Action::kConvertTaper,
            Action::kConvertTwist }
        },
        { ActionCategory::kCreation,      // LCOV_EXCL_LINE [bug in gcovr]
          { Action::kCreateBox,
            Action::kCreateCylinder,
            Action::kCreateExtruded,
            Action::kCreateImportedModel,
            Action::kCreateRevSurf,
            Action::kCreateSphere,
            Action::kCreateText,
            Action::kCreateTorus }
        },
        { ActionCategory::kLayout,        // LCOV_EXCL_LINE [bug in gcovr]
          { Action::kLinearLayout,
            Action::kMoveToOrigin,
            Action::kRadialLayout,
            Action::kToggleAxisAligned,
            Action::kToggleEdgeTarget,
            Action::kTogglePointTarget }
        },
        { ActionCategory::kModification,  // LCOV_EXCL_LINE [bug in gcovr]
          { Action::kDecreaseComplexity,
            Action::kIncreaseComplexity,
            Action::kMoveNext,
            Action::kMovePrevious }
        },
        { ActionCategory::kNone,          // LCOV_EXCL_LINE [bug in gcovr]
          { Action::kNone }
        },
        { ActionCategory::kPrecision,     // LCOV_EXCL_LINE [bug in gcovr]
          { Action::kDecreasePrecision,
            Action::kIncreasePrecision }
        },
        { ActionCategory::kRadialMenu,    // LCOV_EXCL_LINE [bug in gcovr]
          { Action::kToggleLeftRadialMenu,
            Action::kToggleRightRadialMenu }
        },
        { ActionCategory::kSelection,     // LCOV_EXCL_LINE [bug in gcovr]
          { Action::kSelectAll,
            Action::kSelectFirstChild,
            Action::kSelectNextSibling,
            Action::kSelectNone,
            Action::kSelectParent,
            Action::kSelectPreviousSibling }
        },
        { ActionCategory::kSession,       // LCOV_EXCL_LINE [bug in gcovr]
          { Action::kOpenHelpPanel,
            Action::kOpenInfoPanel,
            Action::kOpenSessionPanel,
            Action::kOpenSettingsPanel }
        },
        { ActionCategory::kSpecialized,   // LCOV_EXCL_LINE [bug in gcovr]
          { Action::kToggleSpecializedTool }
        },
        { ActionCategory::kTool,          // LCOV_EXCL_LINE [bug in gcovr]
          { Action::kColorTool,
            Action::kComplexityTool,
            Action::kNameTool,
            Action::kRotationTool,
            Action::kScaleTool,
            Action::kSwitchToNextTool,
            Action::kSwitchToPreviousTool,
            Action::kTranslationTool }
        },
        { ActionCategory::kViewing,       // LCOV_EXCL_LINE [bug in gcovr]
          { Action::kHideSelected,
            Action::kShowAll,
            Action::kToggleBuildVolume,
            Action::kToggleInspector,
            Action::kToggleShowEdges }
        },
    });
    return map;
}

ActionMap::ActionMap_ ActionMap::BuildActionMap_(
    const CategoryMap_ &category_map) {

    ActionMap_ action_map;

    for (const auto &pair: category_map) {
        for (const auto &action: pair.second)
            action_map[action] = pair.first;
    }

#if ENABLE_DEBUG_FEATURES
    // Ensure that every action has a category.
    for (const Action action: Util::EnumValues<Action>()) {
        if (action == Action::kReloadScene)
            continue;
        ASSERTM(action_map.contains(action), Util::EnumName(action));
    }
#endif

    return action_map;
}
