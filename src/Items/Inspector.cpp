#include "Items/Inspector.h"

#include "Base/Tuning.h"
#include "Items/Controller.h"
#include "Math/Linear.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"

void Inspector::Activate(const SG::NodePtr &node,
                         const ControllerPtr &controller) {
    ASSERT(! IsEnabled());

    // Add the inspected Node to the parent and translate the parent to
    // center it on the origin.
    parent_->SetTranslation(-node->GetTranslation());
    parent_->AddChild(node);

    if (controller) {
        // Attach the parent node to the Controller.
        parent_->SetRotation(
            Rotationf::FromAxisAndAngle(Vector3f::AxisX(),
                                        Anglef::FromDegrees(-90)));
        controller->AttachObject(parent_, TK::kInspectorVRFraction,
                                 Vector3f(TK::kInspectorVRXOffset, 0, 0));
        attached_controller_ = controller;
    }
    else {
        // Compute a reasonable scale.
        const Vector3f size = node->GetScaledBounds().GetSize();
        const float scale =
            TK::kInspectorNonVRModelSize / size[GetMaxElementIndex(size)];
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
        attached_controller_->DetachObject(parent_);
        attached_controller_.reset();

        // Reset the change to the parent's transform from the attachment.
        parent_->SetRotation(Rotationf::Identity());
        parent_->SetUniformScale(1);
        parent_->SetTranslation(Vector3f::Zero());
    }
    parent_->ClearChildren();

    SetEnabled(false);

    if (deactivation_func_)
        deactivation_func_();
}

void Inspector::ApplyScaleChange(float delta) {
    ASSERT(IsEnabled());
    if (! attached_controller_) {
        const float cur_scale = transformer_->GetScale()[0];
        const float scale =
            Clamp((1 + TK::kInspectorNonVRScaleMult * delta) * cur_scale,
                  TK::kInspectorNonVRMinScale, TK::kInspectorNonVRMaxScale);
        transformer_->SetUniformScale(scale);
    }
}

void Inspector::ApplyRotation(const Rotationf &rot) {
    ASSERT(transformer_);
    if (! attached_controller_)
        transformer_->SetRotation(rot);
}

void Inspector::ShowEdges(bool show) {
    auto &block = parent_->GetUniformBlockForPass("Lighting");
    block.SetFloatUniformValue("uEdgeWidth", show ? 1 : 0);
}

void Inspector::PostSetUpIon() {
    // Access important Nodes.
    ASSERT(! transformer_);
    transformer_ = SG::FindNodeUnderNode(*this, "Transformer");
    parent_      = SG::FindNodeUnderNode(*this, "Parent");
}
