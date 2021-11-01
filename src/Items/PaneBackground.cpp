#include "Items/Board.h"

PaneBackground::PaneBackground() {
}

void PaneBackground::AddFields() {
    AddField(color_);
    SG::Node::AddFields();
}

void PaneBackground::PostSetUpIon() {
    SG::Node::PostSetUpIon();
    SetBaseColor(color_);
}

