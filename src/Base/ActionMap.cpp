#include "Base/ActionMap.h"

#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/General.h"

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
        { ActionCategory::kBasics,
          { Action::kCopy,
            Action::kCut,
            Action::kDelete,
            Action::kPaste,
            Action::kPasteInto,
            Action::kQuit,
            Action::kRedo,
            Action::kUndo }
        },
        { ActionCategory::kCombination,
          { Action::kCombineCSGDifference,
            Action::kCombineCSGIntersection,
            Action::kCombineCSGUnion,
            Action::kCombineHull }
        },
        { ActionCategory::kConversion,
          { Action::kConvertBevel,
            Action::kConvertClip,
            Action::kConvertMirror }
        },
        { ActionCategory::kCreation,
          { Action::kCreateBox,
            Action::kCreateCylinder,
            Action::kCreateImportedModel,
            Action::kCreateRevSurf,
            Action::kCreateSphere,
            Action::kCreateText,
            Action::kCreateTorus }
        },
        { ActionCategory::kLayout,
          { Action::kLinearLayout,
            Action::kMoveToOrigin,
            Action::kRadialLayout,
            Action::kToggleAxisAligned,
            Action::kToggleEdgeTarget,
            Action::kTogglePointTarget }
        },
        { ActionCategory::kModification,
          { Action::kDecreaseComplexity,
            Action::kIncreaseComplexity,
            Action::kMoveNext,
            Action::kMovePrevious }
        },
        { ActionCategory::kNone,
          { Action::kNone }
        },
        { ActionCategory::kPrecision,
          { Action::kDecreasePrecision,
            Action::kIncreasePrecision }
        },
        { ActionCategory::kRadialMenu,
          { Action::kToggleLeftRadialMenu,
            Action::kToggleRightRadialMenu }
        },
        { ActionCategory::kSelection,
          { Action::kSelectAll,
            Action::kSelectFirstChild,
            Action::kSelectNextSibling,
            Action::kSelectNone,
            Action::kSelectParent,
            Action::kSelectPreviousSibling }
        },
        { ActionCategory::kSession,
          { Action::kOpenHelpPanel,
            Action::kOpenInfoPanel,
            Action::kOpenSessionPanel,
            Action::kOpenSettingsPanel }
        },
        { ActionCategory::kSpecialized,
          { Action::kToggleSpecializedTool }
        },
        { ActionCategory::kTool,
          { Action::kColorTool,
            Action::kComplexityTool,
            Action::kNameTool,
            Action::kRotationTool,
            Action::kScaleTool,
            Action::kSwitchToNextTool,
            Action::kSwitchToPreviousTool,
            Action::kTranslationTool }
        },
        { ActionCategory::kViewing,
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
        ASSERTM(Util::MapContains(action_map, action), Util::EnumName(action));
    }
#endif

    return action_map;
}
