#include "Panels/TaperToolPanel.h"

#include "Panes/TaperProfilePane.h"
#include "Panes/RadioButtonPane.h"
#include "Util/Enum.h"

void TaperToolPanel::InitInterface() {
    const auto &root_pane = GetPane();
    profile_pane_ = root_pane->FindTypedPane<TaperProfilePane>("Profile");

    // Detect changes to everything.
    profile_pane_->GetActivation().AddObserver(
        this, [&](bool is_act){ Activate_("Profile", is_act); });
    profile_pane_->GetProfileChanged().AddObserver(
        this, [&](const Profile &){ Change_("Profile"); });

    // Set up radio buttons.
    std::vector<RadioButtonPanePtr> buts;
    buts.push_back(root_pane->FindTypedPane<RadioButtonPane>("X"));
    buts.push_back(root_pane->FindTypedPane<RadioButtonPane>("Y"));
    buts.push_back(root_pane->FindTypedPane<RadioButtonPane>("Z"));
    RadioButtonPane::CreateGroup(buts, 0);

    // Detect changes to the axis.
    auto set_axis = [&](size_t index){
        const Axis new_axis = Util::EnumFromIndex<Axis>(index);
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
    const std::string axis_name = Util::EnumToWord(axis_);
    GetPane()->FindTypedPane<RadioButtonPane>(axis_name)->SetState(true);
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
    ClickableWidgetPtr widget;

    // XXXX Maybe radio buttons?

    return profile_pane_->GetGripWidget(panel_point);
}

void TaperToolPanel::Activate_(const std::string &key, bool is_activation) {
    is_dragging_ = is_activation;
    ReportChange(key, is_activation ? InteractionType::kDragStart :
                 InteractionType::kDragEnd);
}

void TaperToolPanel::Change_(const std::string &key) {
    ReportChange(key, is_dragging_ ? InteractionType::kDrag :
                 InteractionType::kImmediate);
}
