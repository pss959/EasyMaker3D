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

// Recursive function that prints node matrices.
static void PrintNodeMatrices_(const SG::NodePtr &node, int level,
                               const Matrix4f &start_matrix) {
    const std::string indent(4 * level, ' ');
    const std::string name = node->GetName();
    std::cout << indent << "Node ";
    if (! name.empty())
        std::cout << '"' << name << "\" ";
    std::cout << node->GetModelMatrix() << "\n";

    const Matrix4f combined = start_matrix * node->GetModelMatrix();
    std::cout << indent << "   => " << combined << "\n";

    for (const auto &child: node->GetChildren())
        PrintNodeMatrices_(child, level + 1, combined);
}

// ----------------------------------------------------------------------------
// ShortcutHandler functions.
// ----------------------------------------------------------------------------

bool ShortcutHandler::HandleEvent(const Event &event) {
    // Handle special key presses.
    if (event.flags.Has(Event::Flag::kKeyPress)) {

        // Escape key: quit!
        if (event.key_string == "Escape") {
            action_manager_->ApplyAction(Action::kQuit);
            return true;
        }

#if XXXX // Figure this out...
        // Ctrl-B: Print bounds.
        if (event.key_string == "<Ctrl>b") {
            std::cout << "--------------------------------------------------\n";
            PrintNodeBounds_(scene->GetRootNode(), 0);
            std::cout << "--------------------------------------------------\n";
            return true;
        }

        // Ctrl-M: Print matrices.
        if (event.key_string == "<Ctrl>m") {
            std::cout << "--------------------------------------------------\n";
            PrintNodeMatrices_(scene->GetRootNode(), 0, Matrix4f::Identity());
            std::cout << "--------------------------------------------------\n";
            return true;
        }

        // Ctrl-P: Print scene contents.
        if (event.key_string == "<Ctrl>p") {
            std::cout << "--------------------------------------------------\n";
            Parser::Writer writer;
            writer.SetAddressFlag(true);
            writer.WriteObject(scene, std::cout);
            std::cout << "--------------------------------------------------\n";
            return true;
        }

        // Ctrl-R: Reload the scene.
        else if (event.key_string == "<Ctrl>r") {
            app_.ReloadScene();
            return true;
        }
#endif

        // Ctrl-Z: Undo.
        else if (event.key_string == "<Ctrl>z") {
            if (action_manager_->CanApplyAction(Action::kUndo))
                action_manager_->ApplyAction(Action::kUndo);
            return true;
        }
        // Shift-Ctrl-Z: Redo.
        else if (event.key_string == "<Shift><Ctrl>z") {
            if (action_manager_->CanApplyAction(Action::kRedo))
                action_manager_->ApplyAction(Action::kRedo);
            return true;
        }
    }
    return false;
}
