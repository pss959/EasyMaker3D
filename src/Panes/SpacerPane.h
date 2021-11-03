#pragma once

#include <memory>

#include "Panes/Pane.h"

namespace Parser { class Registry; }

/// SpacerPane is a derived Pane that has no visible presence. It is used to
/// create expanding spaces to separate or right-justify other elements.
class SpacerPane : public Pane {
  protected:
    SpacerPane() {}

  private:
    friend class Parser::Registry;
};

typedef std::shared_ptr<SpacerPane> SpacerPanePtr;
