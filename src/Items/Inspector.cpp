#include "Items/Inspector.h"

#include "Math/Linear.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"

// Constants affecting Inspector behavior.
static const float kTargetSize_ = 8;
static const float kScaleMult_  = .05f;

void Inspector::Activate(const SG::NodePtr &node) {
    ASSERT(! IsEnabled());

    // Access Nodes if not already done.
    if (! transformer_) {
        transformer_ = SG::FindNodeUnderNode(*this, "Transformer");
        parent_      = SG::FindNodeUnderNode(*this, "Parent");
    }

    // Add the inspected Node to the parent and translate the parent to center
    // it on the origin.
    parent_->SetTranslation(-node->GetTranslation());
    parent_->AddChild(node);

    // Compute a reasonable scale.
    const Vector3f size = node->GetScaledBounds().GetSize();
    const float scale = kTargetSize_ / size[GetMaxElementIndex(size)];
    transformer_->SetUniformScale(scale);

    // Reset the rotation.
    transformer_->SetRotation(Rotationf::Identity());

    SetEnabled(true);
}

void Inspector::Deactivate() {
    ASSERT(IsEnabled());
    ASSERT(parent_);
    parent_->ClearChildren();
    SetEnabled(false);

    if (deactivation_func_)
        deactivation_func_();
}

void Inspector::ApplyScaleChange(float delta) {
    ASSERT(IsEnabled());
    const float cur_scale = transformer_->GetScale()[0];
    const float scale = Clamp((1 + kScaleMult_ * delta) * cur_scale, .1f, 10);
    transformer_->SetUniformScale(scale);
}

void Inspector::ApplyRotation(const Rotationf &rot) {
    ASSERT(transformer_);
    transformer_->SetRotation(rot);
}

void Inspector::ShowEdges(bool show) {
    auto &block = GetUniformBlockForPass("Lighting");
    block.SetFloatUniformValue("uEdgeWidth", show ? 1 : 0);
}

const SG::Node * Inspector::GetGrippableNode() const {
    return IsEnabled() ? this : nullptr;
}

void Inspector::UpdateGripInfo(GripInfo &info) {
    /// \todo (VR) Grip
}

void Inspector::ActivateGrip(Hand hand, bool is_active) {
    /// \todo (VR) Grip
}
