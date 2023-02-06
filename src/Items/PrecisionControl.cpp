#include "Items/PrecisionControl.h"

#include "Enums/Action.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "Util/String.h"
#include "Widgets/IconWidget.h"

void PrecisionControl::CreationDone() {
    SG::Node::CreationDone();
    if (! IsTemplate()) {
        text_ = SG::FindTypedNodeUnderNode<SG::TextNode>(*this, "Text");
    }
}

std::vector<IconWidgetPtr> PrecisionControl::GetIcons() const {
    ASSERT(IsCreationDone());

    std::vector<IconWidgetPtr> icons;
    icons.push_back(
        SG::FindTypedNodeUnderNode<IconWidget>(*this, "IncreasePrecision"));
    icons.push_back(
        SG::FindTypedNodeUnderNode<IconWidget>(*this, "DecreasePrecision"));
    return icons;
}

void PrecisionControl::Update(float linear_precision, float angular_precision) {
    text_->SetText(Util::ToString(linear_precision) + " / " +
                   Util::ToString(angular_precision) + "°");
}
