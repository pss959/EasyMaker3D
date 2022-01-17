#include "Panes/Pane.h"

#include "Util/KLog.h"
#include "Util/String.h"

void Pane::AddFields() {
    AddField(base_size_);
    AddField(resize_width_);
    AddField(resize_height_);
    AddField(background_);
    AddField(border_);
    SG::Node::AddFields();
}

void Pane::CreationDone(bool is_template) {
    SG::Node::CreationDone(is_template);

    if (! is_template) {
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
        ProcessSizeChange(*this);
    }
}

const Vector2f & Pane::GetMinSize() const {
    if (min_size_[0] == 0 && min_size_[1] == 0) {
        min_size_ = ComputeMinSize();
        KLOG('p', "MinSize for " << GetDesc() << " = " << min_size_);
    }
    return min_size_;
}

void Pane::SetRectInParent(const Range2f &rect) {
    ASSERTM(! rect.IsEmpty(), "Empty Pane Rect for " + GetDesc());
    rect_in_parent_ = rect;

    SetScale(Vector3f(rect.GetSize(), 1));
    SetTranslation(Vector3f(rect.GetCenter() - Point2f(.5f, .5f),
                            GetTranslation()[2]));
}

void Pane::SetMinSize(const Vector2f &size) {
    if (min_size_ != size) {
        KLOG('p', "MinSize for " << GetDesc() << " now " << size);
        min_size_ = size;
    }
}

void Pane::ProcessSizeChange(const Pane &initiating_pane) {
    size_changed_.Notify();
}

std::string Pane::ToString() const {
    auto tovec2 = [](const Vector3f &v){ return Vector2f(v[0], v[1]); };

    return GetDesc() +
        " SZ="  + Util::ToString(GetSize(), .01f) +
        " MS="  + Util::ToString(GetMinSize(), .01f) +
        " S="   + Util::ToString(tovec2(GetScale()), .01f) +
        " T="   + Util::ToString(tovec2(GetTranslation()), .01f) +
        " RS=[" + Util::ToString(IsWidthResizable(),  true) +
        ","     + Util::ToString(IsHeightResizable(), true) + "]";
}

