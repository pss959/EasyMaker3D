#pragma once

#include <vector>

#include "Panes/BoxPane.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(TouchWrapperPane);
DECL_SHARED_PTR(Widget);

/// TouchWrapperPane is a derived BoxPane that enables touch interaction to
/// work with non-Pane Widgets contained in it.
///
/// \ingroup Panes
class TouchWrapperPane : public BoxPane {
  public:
    /// Redefines this to test Widgets wrapped by this instance.
    virtual WidgetPtr GetTouchedWidget(const TouchInfo &info,
                                       float &closest_distance) override;

  protected:
    TouchWrapperPane() {}

  private:
    friend class Parser::Registry;

    /// Returns a vector of Widgets wrapped by this Node.
    std::vector<WidgetPtr> FindWrappedWidgets_() const;
};
