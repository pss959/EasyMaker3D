#include "ShortcutHandler.h"

#include <iostream>

#include "Event.h"
#include "Parser/Writer.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Shape.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

// Recursive function that prints node bounds.
static void PrintNodeBounds_(const SG::NodePtr &node, int level) {
    const std::string name = node->GetName();
    std::cout << std::string(4 * level, ' ')
              << "Node ";
    if (! name.empty())
        std::cout << '"' << name << "\" ";
    std::cout << node->GetBounds() << "\n";

    for (const auto &shape: node->GetShapes()) {
        std::cout << std::string(4 * (level +  1), ' ')
                  << shape->GetTypeName() << " " << shape->GetBounds() << "\n";
    }

    for (const auto &child: node->GetChildren())
        PrintNodeBounds_(child, level + 1);
}

// ----------------------------------------------------------------------------
// ShortcutHandler functions.
// ----------------------------------------------------------------------------

ShortcutHandler::ShortcutHandler(IApplication &app) : app_(app) {
}

ShortcutHandler::~ShortcutHandler() {
}

bool ShortcutHandler::HandleEvent(const Event &event) {
    // Handle special key presses.
    if (event.flags.Has(Event::Flag::kKeyPress)) {

        // Escape key: quit!
        if (event.key_string == "Escape") {
            app_.ApplyAction(Action::kQuit);
            return true;
        }

        // Ctrl-B: Print bounds.
        if (event.key_string == "<Ctrl>b") {
            std::cout << "--------------------------------------------------\n";
            PrintNodeBounds_(app_.GetContext().scene->GetRootNode(), 0);
            std::cout << "--------------------------------------------------\n";
            return true;
        }

        // Ctrl-P: Print scene contents.
        if (event.key_string == "<Ctrl>p") {
            std::cout << "--------------------------------------------------\n";
            Parser::Writer writer;
            writer.SetAddressFlag(true);
            writer.WriteObject(*app_.GetContext().scene, std::cout);
            std::cout << "--------------------------------------------------\n";
            return true;
        }

        // Ctrl-R: Reload the scene.
        else if (event.key_string == "<Ctrl>r") {
            app_.ReloadScene();
            return true;
        }

        // Ctrl-Z: Undo.
        else if (event.key_string == "<Ctrl>z") {
            if (app_.CanApplyAction(Action::kUndo))
                app_.ApplyAction(Action::kUndo);
            return true;
        }
        // Shift-Ctrl-Z: Redo.
        else if (event.key_string == "<Shift><Ctrl>z") {
            if (app_.CanApplyAction(Action::kRedo))
                app_.ApplyAction(Action::kRedo);
            return true;
        }
    }
    return false;
}
