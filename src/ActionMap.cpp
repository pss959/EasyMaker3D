#include "ActionMap.h"

#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/General.h"

ActionMap::ActionMap() :
    category_map_(BuildCategoryMap_()),
    action_map_(BuildActionMap_(category_map_)) {
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
        { ActionCategory::kFile,
          { Action::kOpenHelpPanel,
            Action::kOpenInfoPanel,
            Action::kOpenSessionPanel,
            Action::kOpenSettingsPanel }
        },
        { ActionCategory::kLayout,
          { Action::kLinearLayout,
            Action::kRadialLayout,
            Action::kToggleAxisAligned,
            Action::kToggleEdgeTarget,
            Action::kTogglePointTarget }
        },
        { ActionCategory::kModification,
          { Action::kDecreaseComplexity,
            Action::kIncreaseComplexity,
            Action::kMoveNext,
            Action::kMovePrevious,
            Action::kMoveToOrigin }
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
        { ActionCategory::kSpecialized,
          { Action::kToggleSpecializedTool }
        },
        { ActionCategory::kTools,
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
#if defined DEBUG
            Action::kReloadScene,
#endif
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

#if DEBUG
    // Ensure that every action has a category.
    for (const Action action: Util::EnumValues<Action>())
        ASSERTM(Util::MapContains(action_map, action), Util::EnumName(action));
#endif

    return action_map;
}

#if XXXX
    ActionMap_ map;

    map[Action::kCopy]                   = ActionCategory::kBasics;
    map[Action::kCut]                    = ActionCategory::kBasics;
    map[Action::kDelete]                 = ActionCategory::kBasics;
    map[Action::kPaste]                  = ActionCategory::kBasics;
    map[Action::kPasteInto]              = ActionCategory::kBasics;
    map[Action::kQuit]                   = ActionCategory::kBasics;
    map[Action::kRedo]                   = ActionCategory::kBasics;
    map[Action::kUndo]                   = ActionCategory::kBasics;

    map[Action::kCombineCSGDifference]   = ActionCategory::kCombination;
    map[Action::kCombineCSGIntersection] = ActionCategory::kCombination;
    map[Action::kCombineCSGUnion]        = ActionCategory::kCombination;
    map[Action::kCombineHull]            = ActionCategory::kCombination;

    map[Action::kConvertBevel]           = ActionCategory::kConversion;
    map[Action::kConvertClip]            = ActionCategory::kConversion;
    map[Action::kConvertMirror]          = ActionCategory::kConversion;

    map[Action::kCreateBox]              = ActionCategory::kCreation;
    map[Action::kCreateCylinder]         = ActionCategory::kCreation;
    map[Action::kCreateRevSurf]          = ActionCategory::kCreation;
    map[Action::kCreateSphere]           = ActionCategory::kCreation;
    map[Action::kCreateText]             = ActionCategory::kCreation;
    map[Action::kCreateTorus]            = ActionCategory::kCreation;
    map[Action::kImportModel]            = ActionCategory::kCreation;

    map[Action::kOpenHelpPanel]          = ActionCategory::kFile;
    map[Action::kOpenInfoPanel]          = ActionCategory::kFile;
    map[Action::kOpenSessionPanel]       = ActionCategory::kFile;
    map[Action::kOpenSettingsPanel]      = ActionCategory::kFile;

    map[Action::kLinearLayout]           = ActionCategory::kLayout;
    map[Action::kRadialLayout]           = ActionCategory::kLayout;
    map[Action::kToggleAxisAligned]      = ActionCategory::kLayout;
    map[Action::kToggleEdgeTarget]       = ActionCategory::kLayout;
    map[Action::kTogglePointTarget]      = ActionCategory::kLayout;

    map[Action::kDecreaseComplexity]     = ActionCategory::kModification;
    map[Action::kEditName]               = ActionCategory::kModification;
    map[Action::kIncreaseComplexity]     = ActionCategory::kModification;
    map[Action::kMoveNext]               = ActionCategory::kModification;
    map[Action::kMovePrevious]           = ActionCategory::kModification;
    map[Action::kMoveToOrigin]           = ActionCategory::kModification;

    map[Action::kNone]                   = ActionCategory::kNone;

    map[Action::kDecreasePrecision]      = ActionCategory::kPrecision;
    map[Action::kIncreasePrecision]      = ActionCategory::kPrecision;

    map[Action::kToggleLeftRadialMenu]   = ActionCategory::kRadialMenu;
    map[Action::kToggleRightRadialMenu]  = ActionCategory::kRadialMenu;

    map[Action::kSelectAll]              = ActionCategory::kSelection;
    map[Action::kSelectFirstChild]       = ActionCategory::kSelection;
    map[Action::kSelectNextSibling]      = ActionCategory::kSelection;
    map[Action::kSelectNone]             = ActionCategory::kSelection;
    map[Action::kSelectParent]           = ActionCategory::kSelection;
    map[Action::kSelectPreviousSibling]  = ActionCategory::kSelection;

    map[Action::kToggleSpecializedTool]  = ActionCategory::kSpecialized;

    map[Action::kColorTool]              = ActionCategory::kTools;
    map[Action::kComplexityTool]         = ActionCategory::kTools;
    map[Action::kRotationTool]           = ActionCategory::kTools;
    map[Action::kScaleTool]              = ActionCategory::kTools;
    map[Action::kSwitchToNextTool]       = ActionCategory::kTools;
    map[Action::kSwitchToPreviousTool]   = ActionCategory::kTools;
    map[Action::kTranslationTool]        = ActionCategory::kTools;

    map[Action::kHideSelected]           = ActionCategory::kViewing;
    map[Action::kInspectSelection]       = ActionCategory::kViewing;
    map[Action::kShowAll]                = ActionCategory::kViewing;
    map[Action::kToggleBuildVolume]      = ActionCategory::kViewing;
    map[Action::kToggleShowEdges]        = ActionCategory::kViewing;
#endif
