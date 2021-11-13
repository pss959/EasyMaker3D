#include "Items/Frame.h"

void Frame::AddFields() {
    AddField(width_);
    AddField(depth_);
    AddField(framed_);
    SG::Node::AddFields();
}

bool Frame::IsValid(std::string &details) {
    if (! SG::Node::IsValid(details))
        return false;

    if (GetObjectType() != Parser::Object::ObjType::kTemplate) {
        if (width_ <= 0 || depth_ <= 0) {
            details = "Non-positive width or depth";
            return false;
        }
    }

    return true;
}

void Frame::PreSetUpIon() {
    if (framed_.GetValue()) {
        auto &framed = *framed_.GetValue();
        if (! IsObserving(framed))
            Observe(framed);
    }
}

void Frame::SetFramed(const SG::NodePtr &framed) {
    auto cur_framed = framed_.GetValue();

    if (framed != cur_framed) {
        if (cur_framed)
            Unobserve(*cur_framed);
        framed_ = framed;
        if (framed)
            Observe(*framed);
        ProcessChange(SG::Change::kBounds);  // Have to update bounds.
    }
}

Bounds Frame::UpdateBounds() const {
    if (framed_.GetValue()) {
        // Compute size of the framed object.
        const Vector3f size = framed_.GetValue()->GetScaledBounds().GetSize();
        const float hx = .5f * size[0];
        const float hy = .5f * size[1];
        const float xl = size[0] + 1 * width_;  // Extra long Top/Bottom.

        // Update the 4 frame pieces: Top, Bottom, Left, Right.
        auto set = [this](int index, const std::string &name,
                          float sx, float tx, float ty){
            auto piece = GetChild(index);
            ASSERT(piece->GetName() == name);
            piece->SetScale(Vector3f(sx, width_, depth_));
            piece->SetTranslation(Vector3f(tx, ty, 0));
        };
        set(0, "Top",     xl,        0,  hy);
        set(1, "Bottom",  xl,        0, -hy);
        set(2, "Left",    size[1], -hx,   0);
        set(3, "Right",   size[1],  hx,   0);
    }

    // Let the base class compute the actual bounds.
    return SG::Node::UpdateBounds();
}
