#include "Panes/LabeledSliderPane.h"

#include "Panes/SliderPane.h"

void LabeledSliderPane::AddFields() {
    AddField(range_);
    AddField(precision_);
    AddField(show_min_);
    AddField(show_max_);
    AddField(show_current_);
    AddField(text_color_);
    BoxPane::AddFields();
}

bool LabeledSliderPane::IsValid(std::string &details) {
    if (! BoxPane::IsValid(details))
        return false;
    if (range_.WasSet()) {
        const auto &range = range_.GetValue();
        if (range[0] >= range[1]) {
            details = "Empty or negative range";
            return false;
        }
    }
    return true;
}

void LabeledSliderPane::CreationDone(bool is_template) {
    BoxPane::CreationDone(is_template);

    if (! is_template) {
        min_text_pane_ = FindTypedPane<TextPane>("MinText");
        max_text_pane_ = FindTypedPane<TextPane>("MaxText");
        cur_text_pane_ = FindTypedPane<TextPane>("CurrentText");

        auto slider_pane = FindTypedPane<SliderPane>("Slider");
        slider_pane->GetValueChanged().AddObserver(
            this, [&](float v){ UpdateText_(v); });
    }
}

void LabeledSliderPane::UpdateText_(float new_value) {
    std::cerr << "XXXX new_value = " << new_value << "\n";
}
