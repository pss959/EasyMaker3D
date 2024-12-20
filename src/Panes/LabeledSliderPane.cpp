//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panes/LabeledSliderPane.h"

#include "Panes/SliderPane.h"
#include "Panes/TextPane.h"

void LabeledSliderPane::AddFields() {
    AddField(range_.Init("range",           Vector2f(0, 1)));
    AddField(precision_.Init("precision",   0));
    AddField(text_color_.Init("text_color", Color::Black()));

    BoxPane::AddFields();
}

bool LabeledSliderPane::IsValid(Str &details) {
    if (! BoxPane::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    if (range_.WasSet()) {
        const auto &range = range_.GetValue();
        if (range[0] >= range[1]) {
            details = "Empty or negative range";
            return false;
        }
    }
    return true;
}

void LabeledSliderPane::CreationDone() {
    BoxPane::CreationDone();

    if (! IsTemplate()) {
        // Copy range and precision to the SliderPane.
        auto slider_pane = FindTypedSubPane<SliderPane>("Slider");
        slider_pane->SetRange(range_);
        slider_pane->SetPrecision(precision_);

        // Access the current text pane and add a slider callback to update it.
        cur_text_pane_ = FindTypedSubPane<TextPane>("CurrentText");
        slider_pane->GetValueChanged().AddObserver(
            this, [&](float v){ UpdateText_(v); });
        UpdateText_(slider_pane->GetValue());

        // Access the min/max text panes and set them up.
        auto set_up_text = [&](const Str &name, float val){
            FindTypedSubPane<TextPane>(name)->SetText(Util::ToString(val));
        };
        const auto &range = range_.GetValue();
        set_up_text("MinText", range[0]);
        set_up_text("MaxText", range[1]);
    }
}

SliderPanePtr LabeledSliderPane::GetSliderPane() const {
    return FindTypedSubPane<SliderPane>("Slider");
}

void LabeledSliderPane::UpdateText_(float new_value) {
    cur_text_pane_->SetText(Util::ToString(new_value));
}
