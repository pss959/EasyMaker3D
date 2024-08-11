//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Panes/LeafPane.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(SpacerPane);

/// SpacerPane is a derived LeafPane that has no visible presence. It is used
/// to create expanding spaces to separate or right-justify other elements.
///
/// \ingroup Panes
class SpacerPane : public LeafPane {
  public:
    /// Allow the size to be changed programmatically.
    void SetSpace(const Vector2f &size) { SetMinSize(size); }

  protected:
    SpacerPane() {}

  private:
    friend class Parser::Registry;
};
