#include "Panels/TaperToolPanel.h"

#include "Panes/LabeledSliderPane.h"
#include "Panes/ProfilePane.h"
#include "Panes/SliderPane.h"

void TaperToolPanel::CreationDone() {
    ToolPanel::CreationDone();

    auto &root_pane = GetPane();
    profile_pane_ = root_pane->FindTypedPane<ProfilePane>("Profile");

    auto find_slider = [&](const std::string &name){
        auto lsp = root_pane->FindTypedPane<LabeledSliderPane>(name);
        return lsp->GetSliderPane();
    };
    scale_slider_ = find_slider("ScaleSlider");
    angle_slider_ = find_slider("AngleSlider");

    // Set up the initial slider values.
    scale_slider_->SetValue(1);
    angle_slider_->SetValue(120);

    // Detect changes to everything.
    profile_pane_->GetActivation().AddObserver(
        this, [&](bool is_act){ Activate_("Profile", is_act); });
    profile_pane_->GetProfileChanged().AddObserver(
        this, [&](const Profile &){ Change_("Profile"); });

    scale_slider_->GetActivation().AddObserver(
        this, [&](bool is_act){ Activate_("Scale", is_act); });
    scale_slider_->GetValueChanged().AddObserver(
        this, [&](float){ Change_("Scale"); });

    angle_slider_->GetActivation().AddObserver(
        this, [&](bool is_act){ Activate_("MaxAngle", is_act); });
    angle_slider_->GetValueChanged().AddObserver(
        this, [&](float){ Change_("MaxAngle"); });
}

void TaperToolPanel::SetTaper(const Bevel &taper) {
    // Don't do anything if the Taper has not changed, except for the first
    // time.
    if (! was_taper_set_ || taper != GetTaper()) {
        profile_pane_->GetProfileChanged().EnableObserver(this, false);
        scale_slider_->GetValueChanged().EnableObserver(this, false);
        angle_slider_->GetValueChanged().EnableObserver(this, false);

        profile_pane_->SetProfile(taper.profile);
        scale_slider_->SetValue(taper.scale);
        angle_slider_->SetValue(taper.max_angle.Degrees());

        profile_pane_->GetProfileChanged().EnableObserver(this, true);
        scale_slider_->GetValueChanged().EnableObserver(this, true);
        angle_slider_->GetValueChanged().EnableObserver(this, true);

        was_taper_set_ = true;
    }
}

Bevel TaperToolPanel::GetTaper() const {
    Bevel taper;
    taper.profile   = profile_pane_->GetProfile();
    taper.scale     = scale_slider_->GetValue();
    taper.max_angle = Anglef::FromDegrees(angle_slider_->GetValue());
    return taper;
}

ClickableWidgetPtr TaperToolPanel::GetGripWidget(const Point2f &panel_point) {
    ClickableWidgetPtr widget;

    // If the point is low enough, hover the scale slider.
    const float kLow   = -.2f;
    const float kRight =  .8f;
    if (panel_point[1] <= kLow) {
        widget = scale_slider_->GetActivationWidget();
    }
    // If the point is far right enough, hover the max angle slider.
    else if (panel_point[0] >= kRight) {
        widget = angle_slider_->GetActivationWidget();
    }
    // Otherwise, normalize the point and ask the ProfilePane.
    else {
        // X: 0=>0,    kRight=>1
        // Y: kLow=>0, 1=>1
        const float norm_x =  panel_point[0] / kRight;
        const float norm_y = (panel_point[1] - kLow) / (1 - kLow);
        widget = profile_pane_->GetGripWidget(Point2f(norm_x, norm_y));
    }

    return widget;
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
