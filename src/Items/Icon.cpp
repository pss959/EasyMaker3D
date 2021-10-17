#include "Items/Icon.h"

void Icon::AddFields() {
    AddField(is_toggle_);
    AddField(action_);
    Parser::Object::AddFields();
}
