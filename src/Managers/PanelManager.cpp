#include "Managers/PanelManager.h"

#include "SG/Search.h"
#include "Util/General.h"

void PanelManager::Reset() {
    panel_map_.clear();
}

void PanelManager::FindAllPanels(const SG::Scene &scene,
                                 const Panel::ContextPtr &context) {
    auto add_panel = [this, &scene, &context](const Str &name) {
        auto panel = SG::FindTypedNodeInScene<Panel>(scene, name);
        if (Util::app_type == Util::AppType::kUnitTest)
            panel->SetTestContext(context);
        // LCOV_EXCL_START [only SetTestContext() is used in tests]
        else
            panel->SetContext(context);
        // LCOV_EXCL_STOP
        ASSERTM(! panel_map_.contains(name),
                "Multiple panels with name " + name);
        panel_map_[name] = panel;
    };

    add_panel("ActionPanel");
    add_panel("BevelToolPanel");
    add_panel("CSGToolPanel");
    add_panel("DialogPanel");
    add_panel("ExtrudedToolPanel");
    add_panel("FilePanel");
    add_panel("HelpPanel");
    add_panel("ImportToolPanel");
    add_panel("InfoPanel");
    add_panel("KeyboardPanel");
    add_panel("NameToolPanel");
    add_panel("RadialMenuPanel");
    add_panel("RevSurfToolPanel");
    add_panel("SessionPanel");
    add_panel("SettingsPanel");
    add_panel("TaperToolPanel");
    add_panel("TextToolPanel");
}

PanelPtr PanelManager::GetPanel(const Str &name) const {
    ASSERTM(panel_map_.contains(name), "No panel named " + name);
    return panel_map_.at(name);
}
