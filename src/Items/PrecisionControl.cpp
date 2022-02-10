#include "Items/PrecisionControl.h"

#include "Enums/Action.h"
#include "Managers/ActionManager.h"
#include "SG/Search.h"
#include "Widgets/PushButtonWidget.h"

std::vector<IconWidgetPtr> PrecisionControl::GetIcons() const {
    ASSERT(IsCreationDone());

    std::vector<IconWidgetPtr> icons;
    icons.push_back(
        SG::FindTypedNodeUnderNode<IconWidget>(*this, "IncreasePrecision"));
    icons.push_back(
        SG::FindTypedNodeUnderNode<IconWidget>(*this, "DecreasePrecision"));
    return icons;
}
