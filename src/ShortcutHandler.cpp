#include "ShortcutHandler.h"

#include <iostream>

#include "Event.h"
#include "Interfaces/IViewer.h"
#include "View.h"
#include "SG/Node.h"
#include "SG/Writer.h"

ShortcutHandler::ShortcutHandler(IApplication &app) : app_(app) {
}

ShortcutHandler::~ShortcutHandler() {
}

bool ShortcutHandler::HandleEvent(const Event &event) {
    // Handle special key presses.
    if (event.flags.Has(Event::Flag::kKeyPress)) {

        // Ctrl-P: Print scene contents.
        if (event.key_string == "<Ctrl>p") {
            std::cout << "--------------------------------------------------\n";
            SG::Writer writer;
            writer.WriteScene(*app_.GetContext().scene, std::cout);
            std::cout << "--------------------------------------------------\n";
            return true;
        }

        // Ctrl-R: Reload the scene.
        else if (event.key_string == "<Ctrl>r") {
            app_.ReloadScene();
            return true;
        }

        // Ctrl-V: Print view information.
        else if (event.key_string == "<Ctrl>v") {
            for (const auto viewer: app_.GetContext().viewers) {
                const View &view = viewer->GetView();
                std::cout << "=== View frustum for "
                          << viewer->GetClassName() << ":\n";
                std::cout << view.GetFrustum().ToString() << "\n";
            }
            return true;
        }

    }
    return false;
}
