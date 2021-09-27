#pragma once

#include <memory>

#include "Widgets/ClickableWidget.h"

namespace Parser { class Registry; }

/// PushButtonWidget is a clickable widget that responds to clicks on it,
/// invoking the Notifier.
class PushButtonWidget : public ClickableWidget {
  protected:
    PushButtonWidget() {}

    friend class Parser::Registry;
};

typedef std::shared_ptr<PushButtonWidget> PushButtonWidgetPtr;
