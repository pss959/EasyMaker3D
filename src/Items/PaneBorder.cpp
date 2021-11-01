#include "Items/Board.h"

PaneBorder::PaneBorder() {
}

void PaneBorder::AddFields() {
    AddField(color_);
    AddField(width_);
    SG::Node::AddFields();
}

void PaneBorder::PostSetUpIon() {
    SG::Node::PostSetUpIon();
    GetStateTable()->SetLineWidth(width_);
    SetBaseColor(color_);
}

