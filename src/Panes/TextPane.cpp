#include "Panes/TextPane.h"

#include "Assert.h"
#include "Parser/Registry.h"
#include "SG/TextNode.h"

void TextPane::AddFields() {
    AddField(text_);
    Pane::AddFields();
}

void TextPane::SetText(const std::string &text) {
    text_ = text;
    if (text_node_)
        text_node_->SetText(text_);
}

void TextPane::PreSetUpIon() {
    Pane::PreSetUpIon();
    if (! text_node_ && ! IsTemplate()) {
        text_node_ = Parser::Registry::CreateObject<SG::TextNode>();
        auto opts = Parser::Registry::CreateObject<SG::LayoutOptions>();
        opts->SetHAlignment(SG::LayoutOptions::HAlignment::kAlignHCenter);
        opts->SetVAlignment(SG::LayoutOptions::VAlignment::kAlignVCenter);
        text_node_->SetLayoutOptions(opts);
        AddChild(text_node_);
        SetSize(GetSize());
    }
    SetText(text_);
}

void TextPane::SetSize(const Vector2f &size) {
    Pane::SetSize(size);
    if (text_node_ && (IsWidthResizable() || IsHeightResizable())) {
        auto opts = text_node_->GetLayoutOptions();
        Vector2f target_size = opts->GetTargetSize();
        if (IsWidthResizable())
            target_size[0] = 1;
        if (IsHeightResizable())
            target_size[1] = 1;
        opts->SetTargetSize(target_size);
    }
}
