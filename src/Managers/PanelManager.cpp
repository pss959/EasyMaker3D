#include "Managers/PanelManager.h"

#include <functional>

#include "Items/Board.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"

void PanelManager::Reset() {
    panel_map_.clear();
    default_board_.reset();
    current_board_.reset();
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

    add_panel("ActionPanel");
    add_panel("BevelToolPanel");
    add_panel("CSGToolPanel");
    add_panel("DialogPanel");
    add_panel("FilePanel");
    add_panel("HelpPanel");
    add_panel("ImportToolPanel");
    add_panel("InfoPanel");
    add_panel("NameToolPanel");
    add_panel("RadialMenuPanel");
    add_panel("RevSurfToolPanel");
    add_panel("SessionPanel");
    add_panel("SettingsPanel");
    add_panel("TextToolPanel");
#if DEBUG
    // add_panel("TestPanel");  // Add to help with testing/debugging.
#endif
}

void PanelManager::OpenPanel(const std::string &panel_name) {
    ShowPanel_(FindPanel_(panel_name));
}

void PanelManager::InitAndOpenPanel(const std::string &panel_name,
                                    const InitFunc &init_func) {
    ASSERT(init_func);
    PanelPtr panel = FindPanel_(panel_name);
    KLOG('g', "Initializing " << panel->GetDesc());
    init_func(panel);
    ShowPanel_(panel);
}

void PanelManager::ClosePanel(const std::string &result) {
    Close(result);
}

void PanelManager::Close(const std::string &result) {
    Board &board = GetBoard_();
    PanelPtr cur_panel = board.GetPanel();
    ASSERT(cur_panel);

    KLOG('g', "Closing " << cur_panel->GetDesc()
         << " with result '" << result << "'");

    // Hide the Board to close the current Panel.
    board.Show(false);

    // Restore the previous Panel, if any.
    if (! panel_stack_.empty()) {
        const PanelInfo_ new_panel_info = panel_stack_.top();
        panel_stack_.pop();

        // Show the previous Panel and call its result function, if any.
        KLOG('g', "Reopening " << new_panel_info.panel->GetDesc());
        ShowPanel_(new_panel_info.panel);
        if (new_panel_info.result_func)
            new_panel_info.result_func(*cur_panel, result);
    }
}

void PanelManager::Replace(const std::string &panel_name,
                           const InitFunc &init_func,
                           const ResultFunc &result_func) {
    PanelPtr cur_panel = GetBoard_().GetPanel();
    ASSERT(cur_panel);

    // Push a new PanelInfo_.
    PanelInfo_ info;
    info.panel       = cur_panel;
    info.result_func = result_func;
    panel_stack_.push(info);

    // Let the current Panel initialize the replacement panel and then open it..
    PanelPtr new_panel = FindPanel_(panel_name);
    KLOG('g', "Replacing " << cur_panel->GetDesc()
         << " with " << new_panel->GetDesc());
    if (init_func) {
        KLOG('g', "Initializing " << new_panel->GetDesc());
        init_func(new_panel);
    }
    ShowPanel_(new_panel);
}

PanelPtr PanelManager::FindPanel_(const std::string &name) const {
    ASSERTM(Util::MapContains(panel_map_, name), "No panel named " + name);
    return panel_map_.at(name);
}

void PanelManager::ShowPanel_(const PanelPtr &panel) {
    ASSERT(panel);

    KLOG('g', "Showing " << panel->GetDesc());
    Board &board = GetBoard_();
    board.SetPanel(panel);

    // If using the default Board, make sure it is above the stage, meaning the
    // bottom is above Y=0.
    if (&board == default_board_.get()) {
        const float YOffset = 4;
        Bounds bounds = board.GetBounds();
        Vector3f pos = board.GetTranslation();
        const float min_y = pos[1] + bounds.GetMinPoint()[1];
        if (min_y < 0) {
            pos[1] += YOffset - min_y;
            board.SetTranslation(pos);
        }
    }

    board.Show(true);
}

Board & PanelManager::GetBoard_() const {
    ASSERT(default_board_);
    return current_board_ ? *current_board_ : *default_board_;
}
