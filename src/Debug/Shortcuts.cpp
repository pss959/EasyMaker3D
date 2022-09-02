#include "Debug/Shortcuts.h"

#if ENABLE_DEBUG_FEATURES

#include <iostream>
#include <unordered_map>
#include <vector>

#include "Util/General.h"
#include "Util/Enum.h" // XXXX

namespace {

// ----------------------------------------------------------------------------
// Helper classes.
// ----------------------------------------------------------------------------

/// The ShortcutMap_ class manages debugging shortcuts.
///
/// \ingroup Debug
class ShortcutMap_ {
  public:
    /// Enum representing different actions that are used to help debug
    /// applications.
    enum class Action {
        kNone,
        kPrintBounds,
        kPrintBoundsOnPath,
        kPrintCommands,
        kPrintGraph,
        kPrintGraphOnPath,
        kPrintHelp,
        kPrintIonMatricesOnPath,
        kPrintMatrices,
        kPrintMatricesOnPath,
        kPrintModels,
        kPrintPaneTreeBrief,
        kPrintPaneTreeFull,
        kPrintPosition,
        kPrintSkeleton,
        kPrintSkeletonOnPath,
        kPrintTransforms,
        kPrintTransformsOnPath,
        kPrintView,
        kPrintWidget,
        kReloadScene,
        kToggleEventLogging,
        kToggleLogging,
        kToggleShadows,
        kToggleSphere,
    };

    ShortcutMap_();

    /// Returns the Action corresponding to the given shortcut string, which
    /// may be kNone.
    Action GetAction(const std::string &str) const {
        if (Util::MapContains(action_map_, str))
            return action_map_.at(str);
        else
            return Action::kNone;
    }

    /// Returns the help string to print.
    const std::string & GetHelpString() const { return help_string_; }

  private:
    /// Struct storing info for each Action.
    struct ActionData_ {
        std::string shortcut;
        Action      action;
        std::string help_string;
    };

    /// Returns a vector of ActionData_ instances for all actions.
    static std::vector<ActionData_> GetData_();

    /// Maps shortcut string to an Action.
    std::unordered_map<std::string, Action> action_map_;

    /// Help string printed for the Action_::kPrintHelp shortcut.
    std::string help_string_;

    void InitActionMap_(const std::vector<ActionData_> &data);
    void InitHelpString_(const std::vector<ActionData_> &data);
};

ShortcutMap_::ShortcutMap_() {
    const std::vector<ActionData_> data = GetData_();

    // Set up the map from shortcut string to Action.
    InitActionMap_(data);

    // Set up the help string.
    InitHelpString_(data);
}

std::vector<ShortcutMap_::ActionData_> ShortcutMap_::GetData_() {
    // This vector is sorted by shortcut for the help string.
    return std::vector<ActionData_>{
        { "<Alt>b", Action::kPrintBounds,
          "Print bounds for all nodes in the scene" },
        { "<Alt>B", Action::kPrintBoundsOnPath,
          "Print bounds for all nodes in the current path" },
        { "<Alt>c", Action::kPrintCommands,
          "Print the command list" },
        { "<Alt>d", Action::kPrintPosition,
          "Print the position of the debug sphere" },
        { "<Alt>D", Action::kToggleSphere,
          "Toggle the visibility of the debug sphere" },
        { "<Alt>g", Action::kPrintGraph,
          "Print the full node graph for the scene" },
        { "<Alt>G", Action::kPrintGraphOnPath,
          "Print the full node graph for current path" },
        { "<Alt>h", Action::kPrintHelp,
          "Print this help" },
        { "<Alt>I", Action::kPrintIonMatricesOnPath,
          "Print Ion matrices for all nodes in the current path" },
        { "<Alt>l", Action::kToggleEventLogging,
          "Toggle event logging" },
        { "<Alt>m", Action::kPrintMatrices,
          "Print matrices for all nodes in the scene" },
        { "<Alt>M", Action::kPrintMatricesOnPath,
          "Print matrices for all nodes in the current path" },
        { "<Alt>o", Action::kPrintModels,
          "Print all models in the scene" },
        { "<Alt>p", Action::kPrintPaneTreeBrief,
          "Print a brief pane tree for the current board" },
        { "<Alt>P", Action::kPrintPaneTreeFull,
          "Print the full pane tree for the current board" },
        { "<Alt>r", Action::kReloadScene,
          "Reload the scene from resource files" },
        { "<Alt>n", Action::kPrintSkeleton,
          "Print all nodes and shapes (skeleton) in the scene" },
        { "<Alt>N", Action::kPrintSkeletonOnPath,
          "Print all nodes and shapes (skeleton) on the current path" },
        { "<Alt>s", Action::kToggleShadows,
          "Toggle shadows" },
        { "<Alt>t", Action::kPrintTransforms,
          "Print transforms for all nodes in the scene" },
        { "<Alt>T", Action::kPrintTransformsOnPath,
          "Print transforms for all nodes in the current path" },
        { "<Alt>v", Action::kPrintView,
          "Print the current viewing information" },
        { "<Alt>w", Action::kPrintWidget,
          "Print the widget intersected by the debug sphere" },
        { "<Alt>!", Action::kToggleLogging,
          "Toggle all logging" },
    };
}

void ShortcutMap_::InitActionMap_(const std::vector<ActionData_> &data) {
    for (const auto &d: data)
        action_map_[d.shortcut] = d.action;
}

void ShortcutMap_::InitHelpString_(const std::vector<ActionData_> &data) {
    // Terminal colors.
    const std::string kColor  = "\033[36m";  // Cyan.
    const std::string kNormal = "\033[0m";

    help_string_ = "-----------------------------------------------------\n"
        "Debugging help shortcuts:\n"
        "(Current path is defined by node under mouse cursor.)\n";
    for (const auto &d: data) {
        help_string_ += "   " + kColor + d.shortcut + kNormal +
            ": " + d.help_string + ".\n";
    }
    help_string_ += "-----------------------------------------------------\n\n";
};

static ShortcutMap_ shortcut_map_;

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

namespace Debug {

bool HandleShortcut(const std::string &str) {
    const auto action = shortcut_map_.GetAction(str);
    if (action != ShortcutMap_::Action::kNone) {
        std::cerr << "XXXX Got action " << Util::EnumName(action) << "\n";
        if (action == ShortcutMap_::Action::kPrintHelp) { // XXXX
            std::cout << shortcut_map_.GetHelpString();
        }
        return true;
    }
    /* XXXX
    if (str == "<Alt>!") {
        KLogger::ToggleLogging();
        return true;
    }
    */
    return false;
}

}  // namespace Debug

#endif
