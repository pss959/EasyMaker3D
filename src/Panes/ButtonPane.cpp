#include "Panes/ButtonPane.h"

#include "Parser/Registry.h"
#include "SG/Search.h"

void ButtonPane::PreSetUpIon() {
    // Make sure the button is set up before the ContainerPane tries to add
    // contained panes to it.
    if (! button_) {
        button_ = SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
        button_->GetClicked().AddObserver(this, [this](const ClickInfo &){
            std::cerr << "XXXX " << GetDesc() << " clicked\n";
        });
    }

    BoxPane::PreSetUpIon();
}
