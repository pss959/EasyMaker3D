#pragma once

#include "Memory.h"
#include "Panes/Pane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(SpacerPane);

/// SpacerPane is a derived Pane that has no visible presence. It is used to
/// create expanding spaces to separate or right-justify other elements.
class SpacerPane : public Pane {
  public:
    /// Allow spacer's size to be changed.
    void SetSpace(const Vector2f &size) { SetMinSize(size); }

  protected:
    SpacerPane() {}

  private:
    friend class Parser::Registry;
};
