#pragma once

#include <memory>

#include "Widgets/ClickableWidget.h"

//! PushButtonWidget is a clickable widget that responds to clicks on it,
//! invoking the Notifier.
class PushButtonWidget : public ClickableWidget {
};

typedef std::shared_ptr<PushButtonWidget> PushButtonWidgetPtr;
