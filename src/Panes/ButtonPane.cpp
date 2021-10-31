#include "Panes/ButtonPane.h"

#include "Parser/Registry.h"

void ButtonPane::PreSetUpIon() {
    SolidPane::PreSetUpIon();

    if (! button_) {
        button_ = Parser::Registry::CreateObject<PushButtonWidget>();
        AddChild(button_);
    }
}
