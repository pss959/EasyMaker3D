#include "Panels/ExtrudedToolPanel.h"

#include "Models/ExtrudedModel.h"
#include "Panes/CheckboxPane.h"
#include "Panes/LabeledSliderPane.h"
#include "Panes/ProfilePane.h"
#include "Panes/SliderPane.h"
#include "Panes/TextInputPane.h"
#include "Util/String.h"

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

void ExtrudedToolPanel::InitInterface() {
    auto &root_pane = GetPane();
    snap_checkbox_ = root_pane->FindTypedPane<CheckboxPane>("SnapToPrecision");
    profile_pane_  = root_pane->FindTypedPane<ProfilePane>("Profile");
    sides_text_    = root_pane->FindTypedPane<TextInputPane>("SidesText");
    const auto &ss = root_pane->FindTypedPane<LabeledSliderPane>("SidesSlider");
    sides_slider_  = ss->GetSliderPane();

    // Detect changes to everything.
    snap_checkbox_->GetStateChanged().AddObserver(
        this, [&](){ UpdatePrecision_(); });

    profile_pane_->GetActivation().AddObserver(
        this, [&](bool is_act){ Activate_("Profile", is_act); });
    profile_pane_->GetProfileChanged().AddObserver(
        this, [&](const Profile &){ Change_("Profile"); });

    sides_slider_->SetValue(8);
    sides_text_->SetInitialText("8");

    auto validate = [&](const std::string &s){ return ValidateSidesText_(s); };
    sides_text_->SetValidationFunc(validate);

    sides_slider_->GetValueChanged().AddObserver(
        this, [&](float s){ UpdateSidesFromSlider_(static_cast<size_t>(s)); });

    AddButtonFunc("SetSides", [&](){ SetToPolygon_(); });
}

void ExtrudedToolPanel::UpdatePrecision_() {
    profile_pane_->SetPointPrecision(snap_checkbox_->GetState() ? precision_ :
                                     Vector2f::Zero());
}

bool ExtrudedToolPanel::ValidateSidesText_(const std::string &text) {
    int n;
    if (Util::StringToInteger(text, n) && n >= 3 && n <= 100) {
        sides_slider_->SetValue(n);
        return true;
    }
    return false;
}

void ExtrudedToolPanel::UpdateSidesFromSlider_(size_t sides) {
    sides_text_->SetInitialText(Util::ToString(sides));
}

void ExtrudedToolPanel::SetToPolygon_() {
    const size_t side_count = static_cast<size_t>(sides_slider_->GetValue());
    ASSERT(side_count >= 3U && side_count <= 100U);
    SetProfile(ExtrudedModel::CreateRegularPolygonProfile(side_count));
    ReportChange("Profile", InteractionType::kImmediate);
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
