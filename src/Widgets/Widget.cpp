#include "Widgets/Widget.h"

#include "SG/ColorMap.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

void Widget::AddFields() {
    AddField(inactive_color_.Init("inactive_color"));
    AddField(active_color_.Init("active_color"));
    AddField(disabled_color_.Init("disabled_color"));
    AddField(hover_color_.Init("hover_color"));
    AddField(hover_scale_.Init("hover_scale", Vector3f(1, 1, 1)));
    AddField(tooltip_text_.Init("tooltip_text"));

    SG::Node::AddFields();
}

void Widget::SetInteractionEnabled(bool enabled) {
    if (is_interaction_enabled_ != enabled) {
        is_interaction_enabled_ = enabled;
        KLOG('W', GetDesc() << " now " << (enabled ? "enabled" : "disabled"));
        UpdateColor_();
    }
}

void Widget::SetActive(bool active, bool notify) {
    if (is_interaction_enabled_ && active != is_active_) {
        is_active_ = active;
        KLOG('W', GetDesc() << " now " << (active ? "active" : "inactive"));
        if (hover_count_ > 0)
            StopHovering_();
        UpdateColor_();

        if (notify)
            activation_.Notify(*this, active);
    }
}

void Widget::StartHovering() {
    // Change status only if hovering just started.
    if (! hover_count_++) {
        KLOG('W', GetDesc() << " hover started");
        ChangeHoverState_(true);
        ActivateTooltip_(true);
    }
}

void Widget::StopHovering() {
    ASSERT(hover_count_ > 0);

    // Change status only if hovering just stopped.
    if (hover_count_ == 1U) {
        KLOG('W', GetDesc() << " hover ended");
        StopHovering_();
    }
    else {
        --hover_count_;
    }
}

void Widget::SetInactiveColor(const Color &color) {
    inactive_color_ = color;
    UpdateColor_();
}

void Widget::SetActiveColor(const Color &color) {
    active_color_ = color;
    UpdateColor_();
}

void Widget::PlacePointTarget(const DragInfo &info,
                              Point3f &position, Vector3f &direction,
                              Dimensionality &snapped_dims) {
    ASSERTM(false, "Widget::PlacePointTarget() should not be called");
}

void Widget::PlaceEdgeTarget(const DragInfo &info, float current_length,
                             Point3f &position0, Point3f &position1) {
    ASSERTM(false, "Widget::PlaceEdgeTarget() should not be called");
}

void Widget::PostSetUpIon() {
    SG::Node::PostSetUpIon();
    UpdateColor_();
}

void Widget::StopHovering_() {
    ASSERT(hover_count_ > 0);
    hover_count_ = 0;
    ChangeHoverState_(false);
    ActivateTooltip_(false);
}

void Widget::UpdateColor_() {
    if (ShouldSetBaseColor() && GetIonContext()) {
        if (! is_interaction_enabled_)
            SetBaseColor(GetColor_(disabled_color_, "DisabledColor"));
        else if (is_active_)
            SetBaseColor(GetColor_(active_color_,   "ActiveColor"));
        else
            SetBaseColor(GetColor_(inactive_color_, "InactiveColor"));
    }
}

void Widget::ChangeHoverState_(bool hover) {
    if (hover) {
        // Start hovering only if the widget is enabled. Note that some Widget
        // classes support hovering while active.
        if (is_interaction_enabled_ &&
            (! is_active_ || SupportsActiveHovering())) {
            if (hover_scale_.WasSet()) {
                saved_scale_ = GetScale();
                SetScale(hover_scale_ * saved_scale_);
            }
            SetEmissiveColor(GetColor_(hover_color_, "HoverColor"));
        }
    }
    else {
        if (hover_scale_.WasSet() && saved_scale_ != Vector3f::Zero())
            SetScale(saved_scale_);
        SetEmissiveColor(Color::Clear());
    }
}

Color Widget::GetColor_(const Parser::TField<Color> &field,
                        const std::string &name) const {
    return field.WasSet() ? field :
        SG::ColorMap::SGetColor(color_name_prefix_ + name);
}

void Widget::ActivateTooltip_(bool is_active) {
    const std::string text = tooltip_text_;

    // Nothing to do if there is no tooltip string or tooltip function.
    if (tooltip_func_ && ! text.empty())
        tooltip_func_(*this, text, is_active);
}
