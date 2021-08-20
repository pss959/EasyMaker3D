#include "ShortcutHandler.h"

#include <iostream>

#include <ion/gfxutils/printer.h>

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
        // Shift-Ctrl-P: Print scene's Ion graph.
        if (event.key_string == "<Shift><Ctrl>p") {
            SG::NodePtr root = app_.GetContext().scene->GetRootNode();
            if (root) {
                std::cout << "-----------------------------------------------\n";
                ion::gfxutils::Printer printer;
                printer.EnableAddressPrinting(false);
                printer.EnableFullShapePrinting(false);
                printer.SetFloatCleanTolerance(1e-5f);
                printer.PrintScene(root->GetIonNode(), std::cout);
                std::cout << "-----------------------------------------------\n";
            }
            return true;
        }
        // Ctrl-R: Reload the scene.
        else if (event.key_string == "<Ctrl>r") {
            app_.ReloadScene();
            return true;
        }
    }
    return false;
}
