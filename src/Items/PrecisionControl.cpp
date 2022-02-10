#include "Items/PrecisionControl.h"

#include "Enums/Action.h"
#include "Managers/ActionManager.h"
#include "SG/Search.h"
#include "Widgets/PushButtonWidget.h"

std::vector<IconWidgetPtr> PrecisionControl::InitIcons(
    ActionManager &action_manager) {
    ASSERT(IsCreationDone());

    auto init_icon = [&](const std::string &name, Action action){
        auto icon = SG::FindTypedNodeUnderNode<IconWidget>(*this, name);
        icon->SetEnableFunction([&, action]{
            return action_manager.CanApplyAction(action); });
        icon->GetClicked().AddObserver(
            this, [&, action](const ClickInfo &){
                action_manager.ApplyAction(action);});
        return icon;
    };

    std::vector<IconWidgetPtr> icons;
    icons.push_back(init_icon("IncreasePrecision", Action::kIncreasePrecision));
    icons.push_back(init_icon("DecreasePrecision", Action::kDecreasePrecision));
    return icons;
}
