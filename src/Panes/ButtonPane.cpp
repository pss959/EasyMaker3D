#include "Panes/ButtonPane.h"

#include "Assert.h"
#include "Parser/Registry.h"
#include "SG/Rectangle.h"
#include "Widgets/PushButtonWidget.h"

void ButtonPane::AddFields() {
    // XXXX
    Pane::AddFields();
}

void ButtonPane::PreSetUpIon() {
    if (! GetChildCount()) {
        auto button = Parser::Registry::CreateObject<PushButtonWidget>();
        AddChild(button);
        SG::RectanglePtr rect =
            Parser::Registry::CreateObject<SG::Rectangle>("ButtonRect");
        button->AddShape(rect);
    }
}
