#include "Panes/Pane.h"

#include "Util/KLog.h"
#include "Util/String.h"

void Pane::AddFields() {
    AddField(min_size_);
    AddField(max_size_);
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

        if (min_size_.WasSet())
            size_may_have_changed_ = true;
    }
}

void Pane::SetSize(const Vector2f &size) {
    ASSERTM(size[0] > 0 && size[1] > 0, "for " + GetDesc());
    if (size_ != size) {
        size_ = size;
        KLOG('p', "Size for " << GetDesc() << " now " << size);
    }
    size_may_have_changed_ = false;
}

const Vector2f & Pane::GetBaseSize() const {
    if (size_may_have_changed_) {
        base_size_ = ComputeBaseSize();
        KLOG('p', "Base size for " << GetDesc() << " = " << base_size_);
    }
    return base_size_;
}

void Pane::SizeChanged(const Pane &initiating_pane) {
    if (! size_may_have_changed_) {
        size_may_have_changed_ = true;
        KLOG('p', "SizeChanged for " << GetDesc());
        size_changed_.Notify(initiating_pane);
    }
}

void Pane::SetMinSize(const Vector2f &size) {
    if (min_size_ != size) {
        KLOG('p', "MinSize for " << GetDesc() << " now " << size);
        min_size_ = size;
    }
}

Vector2f Pane::ClampSize(const Pane &pane, const Vector2f &size) {
    const Vector2f &min = pane.GetMinSize();
    const Vector2f &max = pane.GetMaxSize();
    Vector2f clamped = size;
    if (min[0] > 0)
        clamped[0] = std::max(min[0], clamped[0]);
    if (min[1] > 0)
        clamped[1] = std::max(min[1], clamped[1]);
    if (max[0] > 0)
        clamped[0] = std::min(max[0], clamped[0]);
    if (max[1] > 0)
        clamped[1] = std::min(max[1], clamped[1]);
    return clamped;
}

std::string Pane::ToString() const {
    auto tostr2 = [&](const Vector2f &v){ return Util::ToString(v, .01f); };
    auto tostr3 = [&](const Vector3f &v){ return tostr2(Vector2f(v[0], v[1])); };

    return GetDesc() +
        " SZ="  + tostr2(GetSize()) +
        " MN="  + tostr2(GetMinSize()) +
        " MX="  + tostr2(GetMaxSize()) +
        " BS="  + tostr2(GetBaseSize()) +
        " SC="  + tostr3(GetScale()) +
        " TR="  + tostr3(GetTranslation()) +
        " RS=[" + Util::ToString(IsWidthResizable(),  true) +
        ","     + Util::ToString(IsHeightResizable(), true) + "]";
}
