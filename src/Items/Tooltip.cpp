#include "Items/Tooltip.h"

#include "SG/Search.h"
#include "SG/TextNode.h"
#include "Util/Assert.h"
#include "Util/Delay.h"

float Tooltip::delay_ = Defaults::kTooltipDelay;

std::function<TooltipPtr()> Tooltip::creation_func_;

TooltipPtr Tooltip::Create() {
    ASSERT(creation_func_);
    return creation_func_();
}

void Tooltip::SetText(const std::string &text) {
    auto text_node = SG::FindTypedNodeUnderNode<SG::TextNode>(*this, "Text");
    text_node->SetText(text);
}

void Tooltip::ShowAfterDelay() {
    if (! is_delayed_) {
        if (delay_ > 0) {
            delay_id_ = Util::RunDelayed(delay_, [this](){ SetVisible_(true); });
            is_delayed_ = true;
        }
        else
            SetVisible_(true);
    }
}

void Tooltip::Hide() {
    if (is_delayed_) {
        is_delayed_ = false;
        Util::CancelDelayed(delay_id_);
    }
    SetVisible_(false);
}

void Tooltip::SetVisible_(bool is_visible) {
    SetEnabled(is_visible);
}

Bounds Tooltip::UpdateBounds() const {
    return Bounds();
}
