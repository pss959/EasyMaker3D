#include "Panes/ButtonPane.h"

#include "Parser/Registry.h"

void ButtonPane::PreSetUpIon() {
    // Make sure the button is set up before the ContainerPane tries to add
    // contained panes to it.
    if (! button_) {
        button_ = Parser::Registry::CreateObject<PushButtonWidget>();
        button_->GetClicked().AddObserver(this, [this](const ClickInfo &){
            std::cerr << "XXXX " << GetDesc() << " clicked\n";
        });
        AddChild(button_);
    }

    BoxPane::PreSetUpIon();
}
