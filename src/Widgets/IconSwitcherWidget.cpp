#include "Widgets/IconSwitcherWidget.h"

#include "Util/Assert.h"

void IconSwitcherWidget::AddFields() {
    AddField(index_.Init("index", -1));

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

void IconSwitcherWidget::SetIndexByName(const Str &name) {
    const auto &children = GetChildren();
    for (int i = 0; i < static_cast<int>(children.size()); ++i) {
        if (children[i]->GetName() == name) {
            UpdateIndex_(i, false);
            return;
        }
    }
    ASSERTM(false, "No child with name " + name);
}

void IconSwitcherWidget::FitIntoCube(float size, const Point3f &center) {
    // Enable all children and make them the same size with the origin as
    // center to avoid extra translations.
    for (auto &child: GetChildren()) {
        child->SetEnabled(true);
        FitNodeIntoCube(*child, size, Point3f::Zero());
    }

    IconWidget::FitIntoCube(size, center);

    // Enable children based on the current index.
    UpdateIndex_(index_, true);
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
