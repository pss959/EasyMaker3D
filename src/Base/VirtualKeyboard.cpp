#include "Base/VirtualKeyboard.h"

void VirtualKeyboard::InsertText(const std::string &s) {
    insertion_.Notify(s);
}

void VirtualKeyboard::DeletePreviousChar() {
    deletion_.Notify(false);
}

void VirtualKeyboard::ClearText() {
    deletion_.Notify(true);
}

void VirtualKeyboard::Finish(bool accept) {
    completion_.Notify(accept);
}

