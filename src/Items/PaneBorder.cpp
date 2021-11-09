#include "Items/PaneBorder.h"

#include "Assert.h"

PaneBorder::PaneBorder() {
}

void PaneBorder::AddFields() {
    AddField(color_);
    AddField(width_);
    SG::Node::AddFields();
}

void PaneBorder::PostSetUpIon() {
    SG::Node::PostSetUpIon();
    ASSERT(GetStateTable());
    GetStateTable()->SetLineWidth(width_);
    SetBaseColor(color_);
}

