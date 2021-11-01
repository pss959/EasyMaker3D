#include "Panes/Pane.h"

#include "Assert.h"
#include "Util/String.h"

void Pane::AddFields() {
    AddField(base_size_);
    AddField(resize_width_);
    AddField(resize_height_);
    SG::Node::AddFields();
}

bool Pane::IsValid(std::string &details) {
    if (! SG::Node::IsValid(details))
        return false;

    // Initialize the minimum size with the base size.
    SetMinSize(GetBaseSize());

    return true;
}

void Pane::SetSize(const Vector2f &size) {
    if (size_ != size) {
        size_ = size;
        ProcessChange(SG::Change::kGeometry);
    }
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
