#include "ShortcutHandler.h"

#include <iostream>

#include "Event.h"
#include "Interfaces/IResourceManager.h"
#include "Interfaces/IViewer.h"
#include "Loader.h"
#include "Scene.h"
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
            for (IViewer *viewer: app_context_.viewers) {
                std::cout << " View for " << viewer->GetClassName()
                          << ": ---------------------------------\n";
                viewer->GetView().PrintContents();
            }
            return true;
        }
        // Ctrl-R: Reload the scene.
        else if (event.key_string == "<Ctrl>r") {
            Loader(*app_context_.resource_manager).LoadScene(
                *app_context_.scene);
            return true;
        }
    }
    return false;
}
