//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Items/Frame.h"

Frame::~Frame() {
    if (IsCreationDone() && ! IsTemplate()) {
        if (auto framed = GetFramed())
            Unobserve(*framed);
    }
}

void Frame::AddFields() {
    AddField(width_.Init("width", 1));
    AddField(depth_.Init("depth", 1));
    AddField(framed_.Init("framed"));

    SG::Node::AddFields();
}

bool Frame::IsValid(Str &details) {
    if (! SG::Node::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]

    if (width_ <= 0 || depth_ <= 0) {
        details = "Non-positive width or depth";
        return false;
    }

    return true;
}

void Frame::CreationDone() {
    SG::Node::CreationDone();
    if (! IsTemplate()) {
        if (auto framed = GetFramed())
            Observe(*framed);
    }
}

void Frame::FitToSize(const Vector2f &size) const {
    const float hx = .5f * (size[0] + width_);
    const float hy = .5f * (size[1] + width_);
    const float xl = size[0] + 2 * width_;  // Extra long Top/Bottom.

    // Update the 4 frame pieces: Top, Bottom, Left, Right.
    auto set = [this](int index, const Str &name, float sx, float tx, float ty){
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

Bounds Frame::UpdateBounds() const {
    if (GetFramed()) {
        const Vector3f size = GetFramed()->GetScaledBounds().GetSize();
        FitToSize(Vector2f(size[0], size[1]));
    }
    return SG::Node::UpdateBounds();
}
