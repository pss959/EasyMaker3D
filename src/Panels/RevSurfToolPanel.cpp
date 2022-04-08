#include "Panels/RevSurfToolPanel.h"

#include "Panes/LabeledSliderPane.h"
#include "Panes/ProfilePane.h"
#include "Panes/SliderPane.h"

void RevSurfToolPanel::CreationDone() {
    ToolPanel::CreationDone();

    auto &root_pane = GetPane();
    profile_pane_ = root_pane->FindTypedPane<ProfilePane>("Profile");

    auto lsp = root_pane->FindTypedPane<LabeledSliderPane>("SweepAngleSlider");
    sweep_angle_slider_ = lsp->GetSliderPane();

    // Set up the initial slider value.
    sweep_angle_slider_->SetValue(360);

    // Detect changes to everything.
    profile_pane_->GetActivation().AddObserver(
        this, [&](bool is_act){ Activate_("Profile", is_act); });
    profile_pane_->GetProfileChanged().AddObserver(
        this, [&](const Profile &){ Change_("Profile"); });

    sweep_angle_slider_->GetActivation().AddObserver(
        this, [&](bool is_act){ Activate_("SweepAngle", is_act); });
    sweep_angle_slider_->GetValueChanged().AddObserver(
        this, [&](float){ Change_("SweepAngle"); });
}

void RevSurfToolPanel::SetProfile(const Profile &profile) {
    profile_pane_->GetProfileChanged().EnableObserver(this, false);
    profile_pane_->SetProfile(profile);
    profile_pane_->GetProfileChanged().EnableObserver(this, true);
}

const Profile & RevSurfToolPanel::GetProfile() const {
    return profile_pane_->GetProfile();
}

void RevSurfToolPanel::SetSweepAngle(const Anglef &sweep_angle) {
    sweep_angle_slider_->GetValueChanged().EnableObserver(this, false);
    sweep_angle_slider_->SetValue(sweep_angle.Degrees());
    sweep_angle_slider_->GetValueChanged().EnableObserver(this, true);
}

Anglef RevSurfToolPanel::GetSweepAngle() const {
    return Anglef::FromDegrees(sweep_angle_slider_->GetValue());
}

void RevSurfToolPanel::Activate_(const std::string &key, bool is_activation) {
    is_dragging_ = is_activation;
    ReportChange(key, is_activation ? InteractionType::kDragStart :
                 InteractionType::kDragEnd);
}

void RevSurfToolPanel::Change_(const std::string &key) {
    ReportChange(key, is_dragging_ ? InteractionType::kDrag :
                 InteractionType::kImmediate);
}
