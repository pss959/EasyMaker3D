#include "Panes/ButtonPane.h"

#include "ClickInfo.h"
#include "SG/Search.h"

PushButtonWidget & ButtonPane::GetButton() {
    if (! button_)
        button_ = SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
    return *button_;
}

void ButtonPane::Activate() {
    ASSERT(button_);
    ClickInfo info;
    info.is_long_press     = false;
    info.is_alternate_mode = false;
    info.widget = button_.get();
    button_->Click(info);
}
