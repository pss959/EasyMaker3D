#include "Panes/SolidPane.h"

#include "Assert.h"
#include "Parser/Registry.h"
#include "SG/Rectangle.h"
#include "Widgets/PushButtonWidget.h"

void SolidPane::AddFields() {
    AddField(color_);
    AddField(border_color_);
    AddField(border_width_);
    Pane::AddFields();
}

void SolidPane::PreSetUpIon() {
    if (GetShapes().empty()) {
        AddShape(Parser::Registry::CreateObject<SG::Rectangle>());

        if (border_width_ > 0) {
            std::cerr << "XXXX Border width = " << border_width_ << "\n";
        }
    }
    Pane::PreSetUpIon();
}

void SolidPane::PostSetUpIon() {
    SG::Node::PostSetUpIon();
    if (color_.WasSet()) {
        ASSERT(GetChildCount() > 0);
        GetChildren().back()->SetBaseColor(color_);
    }
}
