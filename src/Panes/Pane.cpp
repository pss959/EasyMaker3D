#include "Panes/Pane.h"

#include "Panes/IPaneInteractor.h"
#include "Util/Assert.h"
#include "Util/KLog.h"
#include "Util/String.h"

void Pane::AddFields() {
    AddField(min_size_);
    AddField(resize_width_);
    AddField(resize_height_);
    AddField(background_);
    AddField(border_);
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

bool Pane::IsInteractive() const {
    return false;
}

bool Pane::IsInteractionEnabled() const {
    ASSERTM(false, "Base class Pane::IsInteractionEnabled() called");
    return false;
}

void Pane::Activate() {
    ASSERTM(false, "Base class Pane::Activate() called");
}

void Pane::TakeFocus() {
    ASSERT(focus_func_);
    focus_func_(*this);
}

std::string Pane::ToString() const {
    auto tostr2 = [&](const Vector2f &v){ return Util::ToString(v, .01f); };
    auto tostr3 = [&](const Vector3f &v){ return tostr2(Vector2f(v[0], v[1])); };
    const std::string base_size_star = base_size_may_have_changed_ ? "*" : "";

    return
        (IsEnabled() ? "" : "x:") +
        GetDesc() +
        (HasBackground() ? " BG" : "") +
        " MS="  + tostr2(GetMinSize()) +
        " BS="  + tostr2(GetCurrentBaseSize()) + base_size_star +
        " LS="  + tostr2(GetLayoutSize()) +
        " SC="  + tostr3(GetScale()) +
        " TR="  + tostr3(GetTranslation()) +
        " RS=[" + Util::ToString(IsWidthResizable(),  true) +
        ","     + Util::ToString(IsHeightResizable(), true) + "]";
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
