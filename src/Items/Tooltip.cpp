#include "Items/Tooltip.h"

#include "Util/Time.h"

float Tooltip::delay_ = Defaults::kTooltipDelay;

bool Tooltip::IsValid(std::string &details) {
    // Always turn off intersections for tooltips.
    SetEnabled(Flag::kIntersectAll, false);

    return TextNode::IsValid(details);
}

void Tooltip::ShowAfterDelay() {
    if (delay_ > 0)
        Util::Delay(delay_, [this](){ SetEnabled(Flag::kRender, true); });
}

void Tooltip::Hide() {
    SetEnabled(Flag::kRender, false);
}
