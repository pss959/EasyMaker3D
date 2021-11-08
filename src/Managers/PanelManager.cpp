#include "Managers/PanelManager.h"

#include <functional>

#include "Assert.h"
#include "SG/Search.h"
#include "Util/General.h"

void PanelManager::FindPanels(const SG::Scene &scene) {
    panel_map_["SessionPanel"] =
        SG::FindTypedNodeInScene<SessionPanel>(scene, "SessionPanel");
}

void PanelManager::Activate(const std::string &panel_name) {
    ASSERT(board_);
    ASSERT(Util::MapContains(panel_map_, panel_name));
    auto &panel = panel_map_[panel_name];
    board_->SetPanel(panel);

    // Detect when the panel is closed.
    panel->SetClosedFunc(std::bind(&PanelManager::PanelClosed_,
                                   this, std::placeholders::_1));

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

void PanelManager::PanelClosed_(const std::string &result) {
    std::cerr << "XXXX Panel result = '" << result << "'\n";
    board_->Show(false);
}
