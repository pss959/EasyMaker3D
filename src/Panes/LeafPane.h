//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Panes/Pane.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(LeafPane);

/// LeafPane is an abstract base class for Pane classes that do not contain
/// other Panes (as opposed to ContainerPane).
///
/// \ingroup Panes
class LeafPane : public Pane {
  protected:
    LeafPane() {}

    /// Redefines this to just return the minimum size.
    virtual Vector2f ComputeBaseSize() const override { return GetMinSize(); }
};
