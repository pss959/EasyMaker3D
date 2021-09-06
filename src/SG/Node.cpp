#include "SG/Node.h"

#include <ion/gfx/shaderinputregistry.h>
#include <ion/math/matrix.h>
#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>

#include "Math/Linear.h"

namespace SG {

Node::Node() {}

Node::Node(const std::string &name) {
    SetName(name);
}

void Node::AddFields() {
    AddField(disabled_flags_);
    AddField(scale_);
    AddField(rotation_);
    AddField(translation_);
    AddField(state_table_);
    AddField(shader_program_);
    AddField(uniform_blocks_);
    AddField(shapes_);
    AddField(children_);
    AddField(interactors_);
}

void Node::SetScale(const ion::math::Vector3f &scale) {
    scale_ = scale;
    ProcessChange_(Change::kTransform);
}

void Node::SetRotation(const ion::math::Rotationf &rotation) {
    rotation_ = rotation;
    ProcessChange_(Change::kTransform);
}

void Node::SetTranslation(const ion::math::Vector3f &translation) {
    translation_ = translation;
    ProcessChange_(Change::kTransform);
}

const Matrix4f & Node::GetModelMatrix() {
    if (! matrices_valid_) {
        UpdateMatrices_();
        matrices_valid_ = true;
    }
    return matrix_;
}

const Bounds & Node::GetBounds() {
    if (! bounds_valid_) {
        UpdateBounds_();
        bounds_valid_ = true;
    }
    return bounds_;
}

void Node::Update() {
    // Each of these updates if necessary.
    GetModelMatrix();
    GetBounds();

    // Enable or disable the Ion node for rendering.
    if (ion_node_)
        ion_node_->Enable(IsEnabled(Flag::kTraversal) &&
                          IsEnabled(Flag::kRender));
}

void Node::UpdateForRenderPass(const std::string &pass_name) {
    for (const auto &block: GetUniformBlocks()) {
        const bool enabled =
            block->GetName().empty() || block->GetName() == pass_name;
        block->GetIonUniformBlock()->Enable(enabled);
    }
}

void Node::SetUpIon(IonContext &context) {
    if (! ion_node_) {
        ion_node_.Reset(new ion::gfx::Node);
        ion_node_->SetLabel(GetName());

        // Check for changes to transform fields.
        if (scale_.WasSet() || rotation_.WasSet() ||
            translation_.WasSet()) {
            ProcessChange_(Change::kTransform);
        }

        if (auto &st = GetStateTable()) {
            st->SetUpIon(context);
            ion_node_->SetStateTable(st->GetIonStateTable());
        }
        if (auto &prog = GetShaderProgram()) {
            prog->SetUpIon(context);
            const auto &ion_prog = prog->GetIonShaderProgram();
            ion_node_->SetShaderProgram(ion_prog);
            // Push the registry on the stack.
            context.registry_stack.push(ion_prog->GetRegistry());
        }
        for (const auto &block: GetUniformBlocks()) {
            block->SetUpIon(context);
            ion_node_->AddUniformBlock(block->GetIonUniformBlock());
        }
        for (const auto &shape: GetShapes()) {
            shape->SetUpIon(context);
            ion_node_->AddShape(shape->GetIonShape());

            // Set up notification.
            shape->GetChanged().AddObserver(
                std::bind(&Node::ProcessChange_, this, std::placeholders::_1));
        }
        for (const auto &child: GetChildren()) {
            child->SetUpIon(context);
            ion_node_->AddChild(child->GetIonNode());

            // Set up notification.
            child->GetChanged().AddObserver(
                std::bind(&Node::ProcessChange_, this, std::placeholders::_1));
        }

        // Restore the previous registry.
        if (GetShaderProgram()) {
            ASSERT(context.registry_stack.top() ==
                   GetShaderProgram()->GetIonShaderProgram()->GetRegistry());
            context.registry_stack.pop();
        }
    }
}

void Node::ProcessChange_(const Change &change) {
    // Any change except appearance should invalidate bounds.
    if (change != Change::kAppearance)
        bounds_valid_ = false;
    if (change == Change::kTransform)
        matrices_valid_ = false;

    // Pass notification to observers.
    changed_.Notify(change);
}

void Node::UpdateMatrices_() {
    matrix_ =
        ion::math::TranslationMatrix(GetTranslation()) *
        ion::math::RotationMatrixH(GetRotation()) *
        ion::math::ScaleMatrixH(GetScale());

    // Don't do the rest of this before SetUpIon() is called.
    if (ion_node_) {
        // Create the uniforms if not already done. Note that we have to set
        // both uModelMatrix (which is used by our shaders) and
        // uModelviewMatrix, which is used by the TextNode shaders.
        if (mm_index_ < 0) {
            auto reg = ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
            mm_index_ = ion_node_->AddUniform(
                reg->Create<ion::gfx::Uniform>("uModelMatrix", matrix_));
            mv_index_ = ion_node_->AddUniform(
                reg->Create<ion::gfx::Uniform>("uModelviewMatrix", matrix_));
            ASSERT(mm_index_ >= 0);
            ASSERT(mv_index_ >= 0);
        }
        else {
            ion_node_->SetUniformValue(mm_index_, matrix_);
            ion_node_->SetUniformValue(mv_index_, matrix_);
        }
    }
}

void Node::UpdateBounds_() {
    // Collect and combine Bounds from all shapes and children.
    bounds_.MakeEmpty();
    for (const auto &shape: GetShapes())
        bounds_.ExtendByRange(shape->GetBounds());
    for (const auto &child: GetChildren())
        bounds_.ExtendByRange(TransformBounds(child->GetBounds(),
                                              child->GetModelMatrix()));
}

}  // namespace SG
