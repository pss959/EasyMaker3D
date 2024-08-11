//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Widgets/GenericWidget.h"

void GenericWidget::StartDrag(const DragInfo &info) {
    DraggableWidget::StartDrag(info);
    SetActive(true);
}

void GenericWidget::EndDrag() {
    DraggableWidget::EndDrag();
    SetActive(false);
}
