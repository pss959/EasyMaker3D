#include "Items/PaneBorder.h"

#include "Util/Assert.h"

PaneBorder::PaneBorder() {
}

void PaneBorder::AddFields() {
    AddField(color_.Init("color", Color::Black()));
    AddField(width_.Init("width", 1));

    SG::Node::AddFields();
}

void PaneBorder::PostSetUpIon() {
    SG::Node::PostSetUpIon();
    ASSERT(GetStateTable());

    const float width = width_;
    if (width <= 0) {
        SetEnabled(false);
    }
    else {
        GetStateTable()->SetLineWidth(width_);
        SetBaseColor(color_);
    }
}
