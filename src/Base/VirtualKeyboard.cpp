#include "Base/VirtualKeyboard.h"

void VirtualKeyboard::SetIsActive(bool flag) {
    const bool was_shown = IsShown_();
    is_active_ = flag;
    const bool is_shown = IsShown_();
    if (show_hide_func_ && is_shown != was_shown)
        show_hide_func_(is_shown);
}

void VirtualKeyboard::SetIsVisible(bool flag) {
    const bool was_shown = IsShown_();
    is_visible_ = flag;
    const bool is_shown = IsShown_();
    if (show_hide_func_ && is_shown != was_shown)
        show_hide_func_(is_shown);
}

void VirtualKeyboard::InsertText(const std::string &s) {
    if (IsShown_())
        insertion_.Notify(s);
}

void VirtualKeyboard::ProcessTextAction(TextAction action) {
    if (IsShown_())
        action_.Notify(action);

}
