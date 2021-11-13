#include "Items/AutoFrame.h"

void AutoFrame::AddFields() {
    AddField(framed_);
    Frame::AddFields();
}

bool AutoFrame::IsValid(std::string &details) {
    if (! Frame::IsValid(details))
        return false;

    if (GetObjectType() != Parser::Object::ObjType::kTemplate) {
        if (! GetFramed()) {
            details = "Missing object to size AutoFrame to";
            return false;
        }
    }

    return true;
}

void AutoFrame::PreSetUpIon() {
    if (GetFramed() && ! IsObserving(*GetFramed()))
        Observe(*GetFramed());
}

void AutoFrame::SetFramed(const SG::NodePtr &framed) {
    auto cur_framed = GetFramed();

    if (framed != cur_framed) {
        if (cur_framed)
            Unobserve(*cur_framed);
        framed_ = framed;
        if (framed)
            Observe(*framed);
        ProcessChange(SG::Change::kBounds);  // Have to update bounds.
    }
}

Bounds AutoFrame::UpdateBounds() const {
    if (GetFramed()) {
        const Vector3f size = GetFramed()->GetScaledBounds().GetSize();
        FitToSize(Vector2f(size[0], size[1]));
    }
    return Frame::UpdateBounds();
}
