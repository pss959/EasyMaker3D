#include "Panes/TextPane.h"

#include "Parser/Registry.h"
#include "SG/TextNode.h"

void TextPane::AddFields() {
    AddField(text_);
    Pane::AddFields();
}

void TextPane::SetText(const std::string &text) {
    text_ = text;
    if (GetChildCount())
        Util::CastToDerived<SG::TextNode>(GetChildren()[0])->SetText(text_);
}

void TextPane::PreSetUpIon() {
    if (! GetChildCount()) {
        SG::TextNodePtr text = Parser::Registry::CreateObject<SG::TextNode>();
        AddChild(text);
    }
    SetText(text_);
}
