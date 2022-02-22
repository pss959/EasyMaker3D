#include "Panels/BevelToolPanel.h"

#include "Panes/LabeledSliderPane.h"

void BevelToolPanel::CreationDone() {
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

void BevelToolPanel::SetBevel(const Bevel &bevel) {
    profile_pane_->GetProfileChanged().EnableObserver(this, false);
    scale_slider_->GetValueChanged().EnableObserver(this, false);
    angle_slider_->GetValueChanged().EnableObserver(this, false);

    profile_pane_->SetProfile(bevel.profile);
    scale_slider_->SetValue(bevel.scale);
    angle_slider_->SetValue(bevel.max_angle.Degrees());

    profile_pane_->GetProfileChanged().EnableObserver(this, true);
    scale_slider_->GetValueChanged().EnableObserver(this, true);
    angle_slider_->GetValueChanged().EnableObserver(this, true);
}

Bevel BevelToolPanel::GetBevel() const {
    Bevel bevel;
    bevel.profile   = profile_pane_->GetProfile();
    bevel.scale     = scale_slider_->GetValue();
    bevel.max_angle = Anglef::FromDegrees(angle_slider_->GetValue());
    return bevel;
}

void BevelToolPanel::Activate_(const std::string &key, bool is_activation) {
    is_dragging_ = is_activation;
    ReportChange(key, is_activation ? InteractionType::kDragStart :
                 InteractionType::kDragEnd);
}

void BevelToolPanel::Change_(const std::string &key) {
    ReportChange(key, is_dragging_ ? InteractionType::kDrag :
                 InteractionType::kImmediate);
}
