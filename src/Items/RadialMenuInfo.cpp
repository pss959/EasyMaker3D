#include "Items/RadialMenuInfo.h"

#include "Parser/Registry.h"
#include "Util/Enum.h"

void RadialMenuInfo::AddFields() {
    AddField(count_.Init("count", Count::kCount8));
    AddField(actions_.Init("actions"));

    Parser::Object::AddFields();
}

void RadialMenuInfo::CreationDone() {
    // Fill in any missing actions.
    const size_t count        = Util::EnumInt(GetCount());
    const size_t action_count = actions_.GetValue().size();
    for (size_t i = action_count; i < count; ++i)
        SetButtonAction(i, Action::kNone);
}

RadialMenuInfoPtr RadialMenuInfo::CreateDefault() {
    return Parser::Registry::CreateObject<RadialMenuInfo>();
}

void RadialMenuInfo::SetButtonAction(size_t index, Action action) {
    ASSERT(index < kMaxCount);
    auto &actions = actions_.GetValue();
    const size_t cur_size = actions.size();
    // Increase size if necessary.
    if (index >= cur_size) {
        actions.resize(index + 1);
        for (size_t i = cur_size; i < index; ++i)
            actions[i] = Util::EnumName(Action::kNone);
        actions[index] = Util::EnumName(action);
        actions_ = actions;
    }
    else {
        actions_.ReplaceValue(index, Util::EnumName(action));
    }
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
