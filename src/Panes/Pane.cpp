#include "Panes/Pane.h"

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
    if (size_ != size) {
        size_ = size;
        ProcessPaneSizeChange(*this);
    }
}

const Vector2f & Pane::GetMinSize() const {
    if (min_size_[0] == 0 && min_size_[1] == 0)
        min_size_ = ComputeMinSize();
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
        min_size_ = size;
        ProcessPaneSizeChange(*this);
    }
}

void Pane::ProcessPaneSizeChange(const Pane &pane) {
    // Pass notification to observers.
    pane_size_changed_.Notify(pane);
}
