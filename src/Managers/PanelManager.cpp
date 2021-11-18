#include "Managers/PanelManager.h"

#include <functional>

#include "Assert.h"
#include "SG/Search.h"
#include "Util/General.h"

void PanelManager::FindPanels(const SG::Scene &scene,
                              const Panel::ContextPtr &context) {
    auto add_panel = [this, &scene, &context](const std::string &name) {
        auto panel = SG::FindTypedNodeInScene<Panel>(scene, name);
        panel->SetContext(context);
        panel_map_[name] = panel;
    };

    add_panel("HelpPanel");
    add_panel("SessionPanel");
}

void PanelManager::OpenPanel(const std::string &panel_name) {
    ASSERT(board_);
    ASSERTM(Util::MapContains(panel_map_, panel_name),
            "No panel named " + panel_name);
    ShowPanel_(panel_map_[panel_name]);
}

void PanelManager::PanelClosed_(Panel::CloseReason reason,
                                const std::string &result) {
    // Hide the Board to close the current Panel.
    board_->Show(false);

    // Replace the Panel if requested. The result string is the name of the new
    // Panel to show.
    switch (reason) {
      case Panel::CloseReason::kReplace:
        OpenPanel(result);
        break;

      case Panel::CloseReason::kReplaceAndRestore:
        open_panels_.push(board_->GetPanel());
        OpenPanel(result);
        break;

      default:
        std::cerr << "XXXX Panel done; result = '" << result << "'\n";
        // Restore the previous Panel, if any.
        if (! open_panels_.empty()) {
            ShowPanel_(open_panels_.top());
            open_panels_.pop();
        }
        break;
    }

}

void PanelManager::ShowPanel_(const PanelPtr &panel) {
    ASSERT(board_);
    ASSERT(panel);

    board_->SetPanel(panel);

    // Detect when the panel is closed.
    panel->SetClosedFunc(std::bind(&PanelManager::PanelClosed_,
                                   this, std::placeholders::_1,
                                   std::placeholders::_2));

    // Make sure the board is above the stage, meaning the bottom is above Y=0.
    const float YOffset = 4;
    Bounds bounds = board_->GetBounds();
    Vector3f pos = board_->GetTranslation();
    const float min_y = pos[1] + bounds.GetMinPoint()[1];
    if (min_y < 0) {
        pos[1] += YOffset - min_y;
        board_->SetTranslation(pos);
    }

    board_->Show(true);
}
