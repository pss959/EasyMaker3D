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

void Pane::SetMinSize(const Vector2f &size) {
    if (min_size_ != size) {
        KLOG('p', "MinSize for " << GetDesc() << " now " << size);
        min_size_ = size;
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

void Pane::SetSizeWithinContainer(const Vector2f &size, const Range2f &rect,
                                  bool offset_forward) {
    // Set the scale and translation first. These will cause notification,
    // which can set the size_may_have_changed_ flag.
    SetScale(Vector3f(rect.GetSize(), 1));
    Vector3f trans(rect.GetCenter() - Point2f(.5f, .5f), GetTranslation()[2]);
    if (offset_forward)
        trans[2] += .1f;
    SetTranslation(trans);

    // Set the size after. This will clear the size_may_have_changed_ flag.
    SetSize(size);
}

const Vector2f & Pane::GetBaseSize() const {
    if (size_may_have_changed_) {
        const Vector2f new_base_size = ComputeBaseSize();
        if (new_base_size != base_size_) {
            KLOG('p', "Base size for " << GetDesc() << " = " << base_size_);
            base_size_ = new_base_size;
        }
    }
    return base_size_;
}

void Pane::SizeChanged(const Pane &initiating_pane) {
    if (! size_may_have_changed_) {
        size_may_have_changed_ = true;
        KLOG('q', "SizeChanged for " << GetDesc());
        size_changed_.Notify(initiating_pane);
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
        (HasBackground() ? " BG" : "") +
        " SZ="  + tostr2(GetSize()) + (size_may_have_changed_ ? "*" : "") +
        " MN="  + tostr2(GetMinSize()) +
        " MX="  + tostr2(GetMaxSize()) +
        " BS="  + tostr2(GetBaseSize()) +
        " SC="  + tostr3(GetScale()) +
        " TR="  + tostr3(GetTranslation()) +
        " RS=[" + Util::ToString(IsWidthResizable(),  true) +
        ","     + Util::ToString(IsHeightResizable(), true) + "]";
}
