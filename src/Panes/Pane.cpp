#include "Panes/Pane.h"

#include <ion/math/vectorutils.h>

#include "Math/ToString.h"
#include "Panes/IPaneInteractor.h"
#include "Util/Assert.h"
#include "Util/KLog.h"
#include "Util/String.h"

void Pane::AddFields() {
    AddField(min_size_.Init("min_size",           Vector2f(1, 1)));
    AddField(resize_width_.Init("resize_width",   false));
    AddField(resize_height_.Init("resize_height", false));
    AddField(background_.Init("background"));
    AddField(border_.Init("border"));

    SG::Node::AddFields();
}

void Pane::CreationDone() {
    SG::Node::CreationDone();

    if (! IsTemplate()) {
        if (auto &background = background_.GetValue())
            GetAuxParent().AddChild(background);
        if (auto border = border_.GetValue())
            GetAuxParent().AddChild(border);

        // Assume the base size needs to be computed at least once.
        BaseSizeChanged();
    }
}

const Vector2f & Pane::GetBaseSize() const {
    if (base_size_may_have_changed_) {
        const Vector2f new_base_size = ComputeBaseSize();
        if (new_base_size != base_size_)
            const_cast<Pane *>(this)->SetBaseSize(new_base_size);
        base_size_may_have_changed_ = false;
    }
    return base_size_;
}

void Pane::SetLayoutSize(const Vector2f &size) {
    ASSERTM(size[0] > 0 && size[1] > 0, "for " + GetDesc());
    if (layout_size_ != size) {
        layout_size_ = size;
        KLOG('p', "Layout size for " << GetDesc() << " now " << size);
    }
}

std::string Pane::ToString(bool is_brief) const {
    auto tostr2 = [&](const Vector2f &v){ return Math::ToString(v, .01f); };
    auto tostr3 = [&](const Vector3f &v){ return tostr2(Vector2f(v[0], v[1])); };
    const std::string base_size_star = base_size_may_have_changed_ ? "*" : "";

    std::string s = (IsEnabled() ? "" : "x:") + GetDesc();

    if (is_brief) {
        // Add the upper-left point and size relative to the parent's
        // rectangle. This is derived from the reverse of the math in
        // ContainerPane::PositionSubPane().

        // Trans = rel_center - (.5,.5)
        // Trans = (UL + COFF) / parent_size - (.5,.5)
        // Trans = (UL + (.5,-.5) * child_size) / parent_size - (.5,.5)
        // Trans + (.5,.5) = (UL + (.5,-.5) * child_size) / parent_size
        // parent_size * (Trans + (.5,.5)) - (.5,-.5) * child_size = UL

        const Vector2f rel_size    = WithoutDimension(GetScale(), 2);
        const Vector2f child_size  = GetLayoutSize();
        const Vector2f parent_size = child_size / rel_size;

        const Vector2f center      = WithoutDimension(GetTranslation(), 2);
        const Vector2f upper_left(
            parent_size[0] * (center[0] + .5f) - .5f * child_size[0],
            parent_size[1] * (center[1] + .5f) + .5f * child_size[1]);
        s += " " + tostr2(upper_left) + " / " + tostr2(rel_size);
    }
    else {
        s +=
            std::string(HasBackground() ? " BG" : "") +
            " MS="  + tostr2(GetMinSize()) +
            " BS="  + tostr2(GetCurrentBaseSize()) + base_size_star +
            " LS="  + tostr2(GetLayoutSize()) +
            " SC="  + tostr3(GetScale()) +
            " TR="  + tostr3(GetTranslation()) +
            " RS=[" + Util::ToString(IsWidthResizable(),  true) +
            ","     + Util::ToString(IsHeightResizable(), true) + "]";
    }
    return s;
}

void Pane::SetMinSize(const Vector2f &size) {
    if (min_size_ != size) {
        KLOG('p', "MinSize for " << GetDesc() << " now " << size);
        min_size_ = size;
        BaseSizeChanged();
    }
}

void Pane::BaseSizeChanged() {
    if (! base_size_may_have_changed_) {
        base_size_may_have_changed_ = true;
        KLOG('q', "Base size changed for " << GetDesc());
        base_size_changed_.Notify();
    }
}

void Pane::SetBaseSize(const Vector2f &new_base_size) {
    base_size_ = new_base_size;
    KLOG('p', "Base size for " << GetDesc() << " now = " << base_size_);
}
