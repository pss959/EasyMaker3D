#include "Panes/ButtonPane.h"

#include "SG/Search.h"

PushButtonWidget & ButtonPane::GetButton() {
    if (! button_)
        button_ = SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
    return *button_;
}
