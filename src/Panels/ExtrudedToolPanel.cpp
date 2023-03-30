#include "Panels/ExtrudedToolPanel.h"

#include "Panes/CheckboxPane.h"
#include "Panes/ProfilePane.h"

void ExtrudedToolPanel::CreationDone() {
    ToolPanel::CreationDone();

    auto &root_pane = GetPane();
    snap_checkbox_ = root_pane->FindTypedPane<CheckboxPane>("SnapToPrecision");
    profile_pane_  = root_pane->FindTypedPane<ProfilePane>("Profile");

    // Detect changes to everything.
    snap_checkbox_->GetStateChanged().AddObserver(
        this, [&](){ UpdatePrecision_(); });

    profile_pane_->GetActivation().AddObserver(
        this, [&](bool is_act){ Activate_("Profile", is_act); });
    profile_pane_->GetProfileChanged().AddObserver(
        this, [&](const Profile &){ Change_("Profile"); });
}

void ExtrudedToolPanel::SetProfile(const Profile &profile) {
    profile_pane_->GetProfileChanged().EnableObserver(this, false);
    profile_pane_->SetProfile(profile);
    profile_pane_->GetProfileChanged().EnableObserver(this, true);
}

const Profile & ExtrudedToolPanel::GetProfile() const {
    return profile_pane_->GetProfile();
}

ClickableWidgetPtr ExtrudedToolPanel::GetGripWidget(const Point2f &panel_point) {
    // Ask the ProfilePane.
    return profile_pane_->GetGripWidget(panel_point);
}

void ExtrudedToolPanel::UpdatePrecision_() {
    profile_pane_->SetPointPrecision(snap_checkbox_->GetState() ? precision_ :
                                     Vector2f::Zero());
}

void ExtrudedToolPanel::Activate_(const std::string &key, bool is_activation) {
    is_dragging_ = is_activation;
    ReportChange(key, is_activation ? InteractionType::kDragStart :
                 InteractionType::kDragEnd);
}

void ExtrudedToolPanel::Change_(const std::string &key) {
    ReportChange(key, is_dragging_ ? InteractionType::kDrag :
                 InteractionType::kImmediate);
}
