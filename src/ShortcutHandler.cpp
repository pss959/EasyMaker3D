#include "ShortcutHandler.h"

#include <iostream>

#include "Event.h"
#include "Interfaces/IViewer.h"
#include "View.h"

ShortcutHandler::ShortcutHandler(const IApplication::Context &app_context) :
    app_context_(app_context) {
}

ShortcutHandler::~ShortcutHandler() {
}

bool ShortcutHandler::HandleEvent(const Event &event) {
    // Handle special key presses.
    if (event.flags.Has(Event::Flag::kKeyPress)) {
        // Ctrl-P: Print scene contents.
        if (event.key_string == "<Ctrl>p") {
            std::cerr << "XXXX Can't print yet!\n";
            /* XXXX
            for (IViewer *viewer: app_context_.viewers) {
                std::cout << " View for " << viewer->GetClassName()
                          << ": ---------------------------------\n";
                viewer->GetView().PrintContents();
            }
            */
            return true;
        }
        // Ctrl-R: Reload the scene.
        else if (event.key_string == "<Ctrl>r") {
            std::cerr << "XXXX Can't reload yet!\n";
            /* XXXX
            Loader(*app_context_.resource_manager).LoadScene(
                *app_context_.scene);
                */
            return true;
        }
    }
    return false;
}
