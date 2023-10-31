#include "Widgets/Widget.h"

#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "Place/TouchInfo.h"
#include "SG/ColorMap.h"
#include "SG/CoordConv.h"
#include "SG/Search.h"
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

Color Widget::GetInactiveColor() const {
    return GetColor_(inactive_color_, "InactiveColor");
}

void Widget::SetActiveColor(const Color &color) {
    active_color_ = color;
    UpdateColor_();
}

Color Widget::GetActiveColor() const {
    return GetColor_(active_color_, "ActiveColor");
}

bool Widget::IsTouched(const TouchInfo &info, float &distance) const {
    // Do nothing if interaction is not enabled.
    if (! IsInteractionEnabled())
        return false;

    // Convert the Widget's bounds into coordinates of the touch sphere.
    ASSERT(info.root_node);
    const SG::NodePath path = SG::FindNodePathUnderNode(info.root_node, *this);
    const auto bounds = TransformBounds(
        GetBounds(), SG::CoordConv(path).GetObjectToRootMatrix());

    // Test for a touch sphere intersection with the bounds.
    float dist;
    KLOG('U', "Testing touch on " << GetDesc() << " with bounds " << bounds);
    if (SphereBoundsIntersect(info.position, info.radius, bounds, dist)) {
        KLOG('U', "  Touched at pos " << info.position << " and dist " << dist);
        distance = dist;
        return true;
    }
    KLOG('U', "  Missed touch at pos " << info.position);

    return false;
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
            SetBaseColor(GetActiveColor());
        else
            SetBaseColor(GetInactiveColor());
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
                        const Str &name) const {
    return field.WasSet() ? field :
        SG::ColorMap::SGetColor(color_name_prefix_ + name);
}

void Widget::ActivateTooltip_(bool is_active) {
    const Str text = tooltip_text_;

    // Nothing to do if there is no tooltip string or tooltip function.
    if (tooltip_func_ && ! text.empty())
        tooltip_func_(*this, text, is_active);
}
