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
    }
}

void Pane::SetSize(const Vector2f &size) {
    ASSERT(size[0] > 0 && size[1] > 0);
    if (size_ != size) {
        size_ = size;
        KLOG('p', "Size for " << GetDesc() << " now " << size);
        size_may_have_changed_ = false;
    }
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
        size_changed_.Notify(initiating_pane);
    }
}

void Pane::SetMinSize(const Vector2f &size) {
    if (min_size_ != size) {
        KLOG('p', "MinSize for " << GetDesc() << " now " << size);
        min_size_ = size;
    }
}

void Pane::SetRectWithinParent(const Range2f &rect) {
    ASSERTM(! rect.IsEmpty(), "Empty Pane Rect for " + GetDesc());
    SetScale(Vector3f(rect.GetSize(), 1));
    SetTranslation(Vector3f(rect.GetCenter() - Point2f(.5f, .5f),
                            GetTranslation()[2]));
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
