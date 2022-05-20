#pragma once

#include <vector>

#include "Base/Memory.h"
#include "SG/Node.h"

namespace Parser { class Registry; }

class ActionManager;

DECL_SHARED_PTR(IconWidget);
DECL_SHARED_PTR(PrecisionControl);

/// The PrecisionControl class manages two precision control buttons on the
/// back wall.
///
/// \ingroup Items
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
