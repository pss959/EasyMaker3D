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
        if (! framed_.GetValue()) {
            details = "Missing framed object";
            return false;
        }
    }
    return true;
}


void Frame::PreSetUpIon() {
    if (GetObjectType() != Parser::Object::ObjType::kTemplate)
        Observe(*framed_.GetValue());
}

Bounds Frame::UpdateBounds() const {
    if (GetObjectType() != Parser::Object::ObjType::kTemplate) {
        // Compute size of the framed object.
        const Vector3f size = framed_.GetValue()->GetBounds().GetSize();
        const float hx = .5f * size[0];
        const float hy = .5f * size[1];
        const float xl = size[0] + 1 * width_;  // Extra long Top/Bottom.
        std::cerr << "XXXX UpdateBounds for " << GetDesc()
                  << " size = " << size << "\n";

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

void Frame::ProcessChange(SG::Change change) {
    std::cerr << "XXXX " << GetDesc()
              << " " << Util::EnumName(change) << "\n";
    SG::Node::ProcessChange(change);
}
