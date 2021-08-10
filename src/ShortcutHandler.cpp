#include "ShortcutHandler.h"

#include "Event.h"
#include "Scene.h"

ShortcutHandler::ShortcutHandler(const IApplication::Context &app_context) :
    app_context_(app_context) {
}

ShortcutHandler::~ShortcutHandler() {
}

bool ShortcutHandler::HandleEvent(const Event &event) {
    // Ctrl-P: Print scene contents.
    if (event.flags.Has(Event::Flag::kKeyPress) &&
        event.key_string == "<Ctrl>p") {
        app_context_.scene->PrintScene();
        return true;
    }
    return false;
}
