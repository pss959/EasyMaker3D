#pragma once

#include <memory>
#include <vector>

#include "SG/Node.h"
#include "Widgets/IconWidget.h"

namespace Parser { class Registry; }

class ActionManager;

/// The PrecisionControl class manages two precision control buttons on the
/// back wall.
class PrecisionControl : public SG::Node {
  public:
    /// Returns a vector containing the IconWidget instances used to modify the
    /// current precision values.
    std::vector<IconWidgetPtr> GetIcons() const;

  protected:
    PrecisionControl() {}

  private:
    friend class Parser::Registry;
};

typedef std::shared_ptr<PrecisionControl> PrecisionControlPtr;
