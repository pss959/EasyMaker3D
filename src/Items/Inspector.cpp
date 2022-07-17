#include "Items/Inspector.h"

#include "Items/Controller.h"
#include "Math/Linear.h"
#include "Parser/Registry.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"

// Constants affecting Inspector behavior.
static const float kTargetSize_ = 14;
static const float kScaleMult_  = .05f;

void Inspector::Activate(const SG::NodePtr &node,
                         const ControllerPtr &controller) {
    ASSERT(! IsEnabled());

    // Access Nodes if not already done.
    if (! transformer_) {
        transformer_ = SG::FindNodeUnderNode(*this, "Transformer");
        parent_      = SG::FindNodeUnderNode(*this, "Parent");
        parent_for_controller_ = Parser::Registry::CreateObject<SG::Node>();
    }

    if (controller) {
        // Attach the node to the Controller, using the special parent Node so
        // that its transform can be modified by the Controller.
        parent_for_controller_->AddChild(node);

        const float kSizeFraction = 1;
        const float kXOffset      = .2f;
        controller->AttachObject(
            parent_for_controller_, kSizeFraction, Vector3f(kXOffset, 0, 0));

        attached_controller_ = controller;
    }
    else {
        // Add the inspected Node to the parent and translate the parent to
        // center it on the origin.
        parent_->SetTranslation(-node->GetTranslation());
        parent_->AddChild(node);

        // Compute a reasonable scale.
        const Vector3f size = node->GetScaledBounds().GetSize();
        const float scale = kTargetSize_ / size[GetMaxElementIndex(size)];
        transformer_->SetUniformScale(scale);

        // Reset the rotation.
        transformer_->SetRotation(Rotationf::Identity());
    }

    SetEnabled(true);
}

void Inspector::Deactivate() {
    ASSERT(IsEnabled());
    ASSERT(parent_);
    if (attached_controller_) {
        attached_controller_->DetachObject(parent_for_controller_);
        parent_for_controller_->ClearChildren();
        attached_controller_.reset();
    }
    else {
        parent_->ClearChildren();
    }
    SetEnabled(false);

    if (deactivation_func_)
        deactivation_func_();
}

void Inspector::ApplyScaleChange(float delta) {
    ASSERT(IsEnabled());
    if (! attached_controller_) {
        const float cur_scale = transformer_->GetScale()[0];
        const float scale =
            Clamp((1 + kScaleMult_ * delta) * cur_scale, .1f, 10);
        transformer_->SetUniformScale(scale);
    }
}

void Inspector::ApplyRotation(const Rotationf &rot) {
    ASSERT(transformer_);
    if (! attached_controller_)
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
