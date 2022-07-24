#include "Items/PaneBackground.h"

PaneBackground::PaneBackground() {
}

void PaneBackground::AddFields() {
    AddField(color_.Init("color"));

    SG::Node::AddFields();
}

void PaneBackground::PostSetUpIon() {
    SG::Node::PostSetUpIon();
    SetBaseColor(color_);
}

