#include "Managers/PanelManager.h"

#include <functional>

#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"

void PanelManager::Reset() {
    panel_map_.clear();
    board_.reset();
    while (! panel_stack_.empty())
        panel_stack_.pop();
}

void PanelManager::FindPanels(const SG::Scene &scene,
                              const Panel::ContextPtr &context) {
    auto add_panel = [this, &scene, &context](const std::string &name) {
        auto panel = SG::FindTypedNodeInScene<Panel>(scene, name);
        panel->SetContext(context);
        ASSERTM(! Util::MapContains(panel_map_, name),
                "Multiple panels with name " + name);
        panel_map_[name] = panel;
    };

    add_panel("DialogPanel");
    add_panel("FilePanel");
    add_panel("HelpPanel");
    add_panel("SessionPanel");
    add_panel("SettingsPanel");
#if DEBUG
    add_panel("TestPanel");
#endif
}

void PanelManager::OpenPanel(const std::string &panel_name) {
    ShowPanel_(FindPanel_(panel_name));
}

void PanelManager::Close(const std::string &result) {
    ASSERT(board_);
    PanelPtr cur_panel = board_->GetPanel();
    ASSERT(cur_panel);

    // Hide the Board to close the current Panel.
    board_->Show(false);

    // Restore the previous Panel, if any.
    if (! panel_stack_.empty()) {
        const PanelInfo_ new_panel_info = panel_stack_.top();
        panel_stack_.pop();

        // Show the previous Panel and call its result function, if any.
        ShowPanel_(new_panel_info.panel);
        if (new_panel_info.result_func)
            new_panel_info.result_func(*cur_panel, result);
    }
}

void PanelManager::Replace(const std::string &panel_name,
                           const InitFunc &init_func,
                           const ResultFunc &result_func) {
    ASSERT(board_);
    PanelPtr cur_panel = board_->GetPanel();
    ASSERT(cur_panel);

    // Push a new PanelInfo_.
    PanelInfo_ info;
    info.panel       = cur_panel;
    info.result_func = result_func;
    panel_stack_.push(info);

    // Let the current Panel initialize the replacement panel and then open it..
    PanelPtr new_panel = FindPanel_(panel_name);
    if (init_func)
        init_func(*new_panel);
    ShowPanel_(new_panel);
}

PanelPtr PanelManager::FindPanel_(const std::string &name) const {
    ASSERT(board_);
    ASSERTM(Util::MapContains(panel_map_, name), "No panel named " + name);
    return panel_map_.at(name);
}

void PanelManager::ShowPanel_(const PanelPtr &panel) {
    ASSERT(board_);
    ASSERT(panel);

    board_->SetPanel(panel);

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
