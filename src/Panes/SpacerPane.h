#pragma once

#include "Memory.h"
#include "Panes/LeafPane.h"

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
