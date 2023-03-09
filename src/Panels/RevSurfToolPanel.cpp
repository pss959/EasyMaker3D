#include "Panels/RevSurfToolPanel.h"

#include "Panes/CheckboxPane.h"
#include "Panes/LabeledSliderPane.h"
#include "Panes/ProfilePane.h"
#include "Panes/SliderPane.h"

void RevSurfToolPanel::CreationDone() {
    ToolPanel::CreationDone();

    auto &root_pane = GetPane();
    snap_checkbox_ = root_pane->FindTypedPane<CheckboxPane>("SnapToPrecision");
    profile_pane_  = root_pane->FindTypedPane<ProfilePane>("Profile");

    auto lsp = root_pane->FindTypedPane<LabeledSliderPane>("SweepAngleSlider");
    sweep_angle_slider_ = lsp->GetSliderPane();

    // Set up the initial slider value.
    sweep_angle_slider_->SetValue(360);

    // Detect changes to everything.
    snap_checkbox_->GetStateChanged().AddObserver(
        this, [&](){ SetSnapToPrecision_(snap_checkbox_->GetState()); });

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

ClickableWidgetPtr RevSurfToolPanel::GetGripWidget(const Point2f &panel_point) {
    ClickableWidgetPtr widget;

    // If the point is low enough, hover the sweep angle slider.
    const float kLow = -.2f;
    if (panel_point[1] <= kLow) {
        widget = sweep_angle_slider_->GetActivationWidget();
    }
    // Otherwise, normalize the point and ask the ProfilePane.
    else {
        // kLow => 0, 1->1
        const float norm_y = (panel_point[1] - kLow) / (1 - kLow);
        widget = profile_pane_->GetGripWidget(Point2f(panel_point[0], norm_y));
    }

    return widget;
}

void RevSurfToolPanel::SetSnapToPrecision_(bool snap) {
    std::cerr << "XXXX STP = " << snap << "\n";
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
