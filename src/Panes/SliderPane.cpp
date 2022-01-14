#include "Panes/SliderPane.h"

#include "SG/Node.h"
#include "SG/Search.h"

void SliderPane::AddFields() {
    AddField(range_);
    AddField(precision_);
    Pane::AddFields();
}

void SliderPane::AllFieldsParsed(bool is_template) {
    Pane::AllFieldsParsed(is_template);
    slider_ = SG::FindTypedNodeUnderNode<Slider1DWidget>(*this, "Slider");
    thumb_  = SG::FindNodeUnderNode(*slider_, "Thumb");
}

float SliderPane::GetValue() const {
    return slider_->GetValue();
}

void SliderPane::SetValue(float value) {
    slider_->SetValue(value);
}

void SliderPane::SetSize(const Vector2f &size) {
    Pane::SetSize(size);

    // Keep the thumb the same relative size.
    // slider_->SetScale(Vector3f(1.f / size[0], 1.f / size[1], 1.f));
    SG::FindNodeUnderNode(*this, "Slider")->SetScale(
        Vector3f(1.f / size[0], 1.f / size[1], 1.f));
}
