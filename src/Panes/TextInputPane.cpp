#include "Panes/TextInputPane.h"

void TextInputPane::AddFields() {
    AddField(initial_text_);
    BoxPane::AddFields();
}

void TextInputPane::PreSetUpIon() {
    BoxPane::PreSetUpIon();
    // XXXX
}

void TextInputPane::PostSetUpIon() {
    BoxPane::PostSetUpIon();
    // XXXX
}

void TextInputPane::Activate() {
    // XXXX Do something.
}
