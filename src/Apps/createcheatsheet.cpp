#include <iostream>
#include <string>

#include "Base/ActionMap.h"
#include "Base/HelpMap.h"
#include "Enums/Action.h"
#include "Enums/ActionCategory.h"
#include "Handlers/ShortcutHandler.h"
#include "Util/Enum.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// Helpers.
// ----------------------------------------------------------------------------

static const char kHeaderString[] =
R"(|appname| Cheat Sheet
=====================

.. This file was generated by the createcheatsheet app.

.. list-table::
   :widths: auto
   :header-rows: 1
   :class: cheat-sheet-table

   * - Category
     - | Name / User
       | Guide Link
     - | Radial Menu
       | Icon
     - Description
     - | Keyboard
       | Shortcut
     - | Controller
       | Shortcut
)";

static void WriteHeader_() {
    std::cout << kHeaderString;
}

static void WriteAction_(ActionCategory cat, Action action,
                         const ShortcutHandler &sh, const HelpMap &hm) {
    const std::string category_name = Util::EnumToWords(cat);
    const std::string action_name   = Util::EnumToWords(action);
    const std::string icon_name     = Util::EnumToWord(action);
    const std::string ref = Util::ReplaceString(
        Util::ToLowerCase(Util::EnumToWords(action)), " ", "-");
    const std::string &desc = hm.GetHelpString(action);

    // Get the shortcuts from the ShortcutHandler.
    std::string kbd_short, contr_short;
    sh.GetShortcutStrings(action, kbd_short, contr_short);

    std::cout
        << "   * - " << category_name << "\n"
        << "     - :ref:`" << action_name << " <ug-" << ref << ">`\n"
        << "     - .. menuicon:: " << icon_name << "\n"
        << "     - " << desc << "\n"
        << "     - " << kbd_short << "\n"
        << "     - " << contr_short << "\n";
}

static void WriteContents_() {
    // Create a ShortcutHandler to access all shortcuts and a HelpMap for
    // accessing tooltips for action descriptions.
    ShortcutHandler sh;
    HelpMap         hm;

    ActionMap action_map;
    for (const auto cat: Util::EnumValues<ActionCategory>()) {
        for (const auto action: action_map.GetActionsInCategory(cat))
            WriteAction_(cat, action, sh, hm);
    }
}

// ----------------------------------------------------------------------------
// Mainline.
// ----------------------------------------------------------------------------

int main(int argc, const char *argv[]) {
    WriteHeader_();
    WriteContents_();
    return 0;
}
