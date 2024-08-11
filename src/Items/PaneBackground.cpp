//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

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

