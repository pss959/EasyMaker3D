//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panes/TouchWrapperPane.h"

#include "Place/TouchInfo.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/General.h"
#include "Widgets/Widget.h"

WidgetPtr TouchWrapperPane::GetTouchedWidget(const TouchInfo &info,
                                             float &closest_distance) {
    // Let the base class test this Pane.
    WidgetPtr best_widget = BoxPane::GetTouchedWidget(info, closest_distance);

    // Find and test all enabled Widgets wrapped by this Pane. Note that the
    // set of wrapped Widgets can change, so storing them in the instance is
    // not that effective.
    for (const auto &widget: FindWrappedWidgets_()) {
        float dist;
        if (widget->IsTouched(info, dist) && dist < closest_distance) {
            closest_distance = dist;
            best_widget = widget;
        }
    }
    return best_widget;
}

std::vector<WidgetPtr> TouchWrapperPane::FindWrappedWidgets_() const {
    const auto is_widget = [](const SG::Node &n){
        if (const Widget *w = dynamic_cast<const Widget *>(&n))
            return w->IsInteractionEnabled();
        else
            return false;
    };

    TouchWrapperPane *cp = const_cast<TouchWrapperPane *>(this);
    SG::NodePtr tp = Util::CreateTemporarySharedPtr<SG::Node>(cp);
    std::vector<WidgetPtr> widgets;
    for (auto &node: SG::FindNodes(tp, is_widget))
        widgets.push_back(std::dynamic_pointer_cast<Widget>(node));
    return widgets;
}
