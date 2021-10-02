#include "Items/Tooltip.h"

#include "Assert.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "Util/Time.h"

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
    if (delay_ > 0)
        Util::Delay(delay_, [this](){ SetVisible_(true); });
}

void Tooltip::Hide() {
    SetVisible_(false);
}

void Tooltip::SetVisible_(bool is_visible) {
    SetEnabled(Flag::kTraversal, is_visible);
}
