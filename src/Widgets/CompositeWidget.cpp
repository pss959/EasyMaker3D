#include "Widgets/CompositeWidget.h"

#include <algorithm>

#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"

CompositeWidget::CompositeWidget() {
}

WidgetPtr CompositeWidget::AddSubWidget(const std::string &name) {
    WidgetPtr widget = SG::FindTypedNodeUnderNode<Widget>(*this, name);

    // Propagate activation notification to observers of the CompositeWidget.
    widget->GetActivation().AddObserver(
        this, [&, name](Widget &, bool is_act){
            SubWidgetActivated(name, is_act); });

    SubWidget_ sub;
    sub.widget         = widget;
    sub.active_color   = widget->GetActiveColor();
    sub.inactive_color = widget->GetInactiveColor();
    sub_widgets_.push_back(sub);

    return widget;
}

void CompositeWidget::SubWidgetActivated(const std::string &name,
                                         bool is_activation) {
    GetActivation().Notify(*this, is_activation);
}

void CompositeWidget::HighlightSubWidget(const std::string &name,
                                         const Color &color) {
    SubWidget_ &sub = FindSubWidget_(name);
    sub.widget->SetActiveColor(color);
    sub.widget->SetInactiveColor(color);
}

void CompositeWidget::UnhighlightSubWidget(const std::string &name) {
    SubWidget_ &sub = FindSubWidget_(name);
    sub.widget->SetActiveColor(sub.active_color);
    sub.widget->SetInactiveColor(sub.inactive_color);
}

CompositeWidget::SubWidget_ & CompositeWidget::FindSubWidget_(
    const std::string &name) {
    SubWidget_ *sub = FindSubWidgetRecursive_(name);
    ASSERTM(sub, "No sub-widget named " + name);
    return *sub;
}

CompositeWidget::SubWidget_ * CompositeWidget::FindSubWidgetRecursive_(
    const std::string &name) {

    const auto matches_name = [&name](const SubWidget_ &sub){
        return sub.widget->GetName() == name;
    };

    // Look in this CompositeWidget.
    const auto it =
        std::find_if(sub_widgets_.begin(), sub_widgets_.end(), matches_name);
    if (it != sub_widgets_.end())
        return &(*it);

    // If not found, look also in sub-widgets that are derived from
    // CompositeWidget.
    for (auto &sub: sub_widgets_) {
        if (auto sub_cw = Util::CastToDerived<CompositeWidget>(sub.widget)) {
            if (auto sw = sub_cw->FindSubWidgetRecursive_(name))
                return sw;
        }
    }

    // If this is reached, it was not found.
    return nullptr;
}
