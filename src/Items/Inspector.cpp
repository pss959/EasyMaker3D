#include "Items/Inspector.h"

#include "Util/Assert.h"

void Inspector::Activate(const SG::NodePtr &node) {
    ASSERT(! IsEnabled());

    // Save the current (default) translation.
    saved_translation_ = GetTranslation();

    // Compute scale and translation to offset the node.
    // XXXX Compute scale.
    SetTranslation(saved_translation_ - node->GetTranslation());

    AddChild(node);
    SetEnabled(true);
}

void Inspector::Deactivate() {
    ASSERT(IsEnabled());

    SetTranslation(saved_translation_);
    ClearChildren();
    SetEnabled(false);
}

bool Inspector::IsGrippableEnabled() const {
    return IsEnabled();
}

void Inspector::UpdateGripInfo(GripInfo &info) {
    // XXXX
}

void Inspector::ActivateGrip(Hand hand, bool is_active) {
    // XXXX
}
