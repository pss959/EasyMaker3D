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

void Pane::SetSize(const Vector2f &size) {
    ASSERT(size[0] > 0 && size[1] > 0);
    if (size_ != size) {
        size_ = size;
        KLOG('p', "Size for " << GetDesc() << " now " << size);
        ProcessSizeChange();
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
    ASSERTM(rect.GetMinPoint()[0] >= 0.f && rect.GetMinPoint()[1] >= 0.f &&
            rect.GetMaxPoint()[0] <= 1.f && rect.GetMaxPoint()[1] <= 1.f &&
            ! rect.IsEmpty(),
            "Bad Pane Rect " + Util::ToString(rect));
    rect_in_parent_ = rect;

    SetScale(Vector3f(rect.GetSize(), 1));
    SetTranslation(Vector3f(rect.GetCenter() - Point2f(.5f, .5f),
                            GetTranslation()[2]));
}

void Pane::PreSetUpIon() {
    SG::Node::PreSetUpIon();

    if (auto &background = background_.GetValue())
        GetAuxParent().AddChild(background);
    if (auto border = border_.GetValue())
        GetAuxParent().AddChild(border);
}

void Pane::SetMinSize(const Vector2f &size) {
    if (min_size_ != size) {
        KLOG('p', "MinSize for " << GetDesc() << " now " << size);
        min_size_ = size;
    }
}

void Pane::ProcessSizeChange() {
    size_changed_.Notify();
}

std::string Pane::ToString() const {
    return GetDesc() +
        " S="  + Util::ToString(GetSize(), .01f)           +
        " MS=" + Util::ToString(GetMinSize(), .01f)        +
        " R=[" + Util::ToString(IsWidthResizable(),  true) +
        ","    + Util::ToString(IsHeightResizable(), true) + "]";
}

