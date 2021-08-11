#include "ShortcutHandler.h"

#include "Event.h"
#include "Scene.h"

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
            app_context_.scene->PrintScene();
            return true;
        }
        // Ctrl-R: Reload the scene.
        else if (event.key_string == "<Ctrl>r") {
            std::cerr << "XXXX Reloading!\n";
            app_context_.scene->Reload();
            return true;
        }
    }
    return false;
}
