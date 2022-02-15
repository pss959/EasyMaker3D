#include "Widgets/IconSwitcherWidget.h"

#include "Util/Assert.h"

void IconSwitcherWidget::AddFields() {
    AddField(index_);
    IconWidget::AddFields();
}

void IconSwitcherWidget::CreationDone() {
    IconWidget::CreationDone();

    // Force update.
    UpdateIndex_(index_, true);
}

void IconSwitcherWidget::SetIndex(int index) {
    UpdateIndex_(index, false);
}

void IconSwitcherWidget::SetIndexByName(const std::string &name) {
    const auto &children = GetChildren();
    for (int i = 0; i < static_cast<int>(children.size()); ++i) {
        if (children[i]->GetName() == name) {
            UpdateIndex_(i, false);
            return;
        }
    }
    ASSERTM(false, "No child with name " + name);
}

void IconSwitcherWidget::UpdateIndex_(int new_index, bool force_update) {
    const int index_to_use =
        new_index < 0 || static_cast<size_t>(new_index) >= GetChildCount() ?
        -1 : new_index;

    // Avoid notifying if not really changing the index.
    if (force_update || GetIndex() != index_to_use) {
        index_ = index_to_use;

        // Enable or disable all children.
        const auto &children = GetChildren();
        for (int i = 0; i < static_cast<int>(children.size()); ++i)
            children[i]->SetEnabled(i == index_to_use);
    }
}
