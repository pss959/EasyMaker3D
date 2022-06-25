#include "Base/VirtualKeyboard.h"

void VirtualKeyboard::InsertText(const std::string &s) {
    if (ShouldNotify_())
        insertion_.Notify(s);
}

void VirtualKeyboard::DeletePreviousChar() {
    if (ShouldNotify_())
        deletion_.Notify(false);
}

void VirtualKeyboard::ClearText() {
    if (ShouldNotify_())
        deletion_.Notify(true);
}

void VirtualKeyboard::Finish(bool accept) {
    if (ShouldNotify_())
        completion_.Notify(accept);
}

