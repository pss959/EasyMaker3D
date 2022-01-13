#include "RadialMenuInfo.h"

#include <vector>

#include "Parser/Registry.h"
#include "Util/Enum.h"

void RadialMenuInfo::AddFields() {
    AddField(count_);
    AddField(actions_);
    Parser::Object::AddFields();
}

void RadialMenuInfo::SetButtonAction(size_t index, Action action) {
    ASSERT(index < kMaxCount);
    auto &actions = actions_.GetValue();
    const size_t cur_size = actions.size();
    if (index >= cur_size) {
        actions.resize(index + 1);
        for (size_t i = cur_size; i < index; ++i)
            actions[i] = Util::EnumName(Action::kNone);
    }
    actions[index] = Util::EnumName(action);
}

Action RadialMenuInfo::GetButtonAction(size_t index) const {
    ASSERT(index < kMaxCount);
    const auto &actions = actions_.GetValue();
    if (index >= actions.size())
        return Action::kNone;
    Action action;
    ASSERT(Util::EnumFromString(actions[index], action));
    Util::EnumFromString(actions[index], action);
    return action;
}
