#include "Items/Inspector.h"

void Inspector::Activate(const SG::NodePtr &node) {
    // XXXX Compute scale.
    AddChild(node);
    SetEnabled(true);
}

void Inspector::Deactivate() {
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
