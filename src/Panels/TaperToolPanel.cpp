//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panels/TaperToolPanel.h"

#include "Math/Linear.h"
#include "Panes/TaperProfilePane.h"
#include "Panes/RadioButtonPane.h"
#include "Util/Enum.h"

void TaperToolPanel::InitInterface() {
    const auto &root_pane = GetPane();
    profile_pane_ = root_pane->FindTypedSubPane<TaperProfilePane>("Profile");

    // Detect changes to everything.
    profile_pane_->GetActivation().AddObserver(
        this, [&](bool is_act){ Activate_("Profile", is_act); });
    profile_pane_->GetProfileChanged().AddObserver(
        this, [&](){ Change_("Profile"); });

    // Set up radio buttons.
    std::vector<RadioButtonPanePtr> buts;
    buts.push_back(root_pane->FindTypedSubPane<RadioButtonPane>("X"));
    buts.push_back(root_pane->FindTypedSubPane<RadioButtonPane>("Y"));
    buts.push_back(root_pane->FindTypedSubPane<RadioButtonPane>("Z"));
    RadioButtonPane::CreateGroup(buts, 0);

    // Detect changes to the axis.
    auto set_axis = [&](size_t index){
        const Dim new_axis = FromUserDim(Util::EnumFromIndex<Dim>(index));
        if (new_axis != axis_) {
            axis_ = new_axis;
            ReportChange("Axis", InteractionType::kImmediate);
        }
    };
    for (auto &but: buts)
        but->GetStateChanged().AddObserver(this, set_axis);
}


void TaperToolPanel::UpdateInterface() {
    // Turn on the correct radio button.
    const Str axis_name = Util::EnumToWord(ToUserDim(axis_));
    GetPane()->FindTypedSubPane<RadioButtonPane>(axis_name)->SetState(true);
}

void TaperToolPanel::SetTaper(const Taper &taper) {
    // Don't do anything if the Taper has not changed, except for the first
    // time.
    if (! was_taper_set_ || taper != GetTaper()) {
        profile_pane_->GetProfileChanged().EnableObserver(this, false);
        profile_pane_->SetProfile(taper.profile);
        profile_pane_->GetProfileChanged().EnableObserver(this, true);

        axis_ = taper.axis;

        was_taper_set_ = true;
    }
}

Taper TaperToolPanel::GetTaper() const {
    Taper taper;
    taper.axis = axis_;
    taper.profile = profile_pane_->GetProfile();
    return taper;
}

ClickableWidgetPtr TaperToolPanel::GetGripWidget(const Point2f &panel_point) {
    return profile_pane_->GetGripWidget(panel_point);
}

void TaperToolPanel::Activate_(const Str &key, bool is_activation) {
    is_dragging_ = is_activation;
    ReportChange(key, is_activation ? InteractionType::kDragStart :
                 InteractionType::kDragEnd);
}

void TaperToolPanel::Change_(const Str &key) {
    ReportChange(key, is_dragging_ ? InteractionType::kDrag :
                 InteractionType::kImmediate);
}
