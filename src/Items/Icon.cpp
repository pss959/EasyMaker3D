#include "Items/Icon.h"

void Icon::AddFields() {
    AddField(is_toggle_.Init("is_toggle", false));
    AddField(action_.Init("action", Action::kNone));

    Parser::Object::AddFields();
}
