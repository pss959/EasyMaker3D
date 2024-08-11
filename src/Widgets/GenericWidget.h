//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"
#include "Util/Memory.h"
#include "Widgets/DraggableWidget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(GenericWidget);

/// A GenericWidget does nothing but pass click and drag events to callers via
/// the GetClicked() and GetDragged() Notifiers. It can have any geometry as
/// children.
///
/// \ingroup Widgets
class GenericWidget : public DraggableWidget {
  public:
    virtual void StartDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

  protected:
    GenericWidget() {}

  private:
    friend class Parser::Registry;
};
