#include "SG/Node.h"

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

void Node::SetBaseColor(const Color &color) {
    GetUniformBlockForPass(PassType::kLightingPass, true)->SetBaseColor(color);
    ProcessChange_(Change::kAppearance);
}

void Node::SetEmissiveColor(const Color &color) {
    GetUniformBlockForPass(
        PassType::kLightingPass, true)->SetEmissiveColor(color);
    ProcessChange_(Change::kAppearance);
}

UniformBlockPtr Node::GetUniformBlockForPass(PassType pass_type,
                                             bool create_if_missing) {
    for (auto &block: GetUniformBlocks()) {
        // This has to be an exact match.
        if (block->GetPassType() == pass_type)
            return block;
    }
    UniformBlockPtr block;
    if (create_if_missing) {
        auto &reg = registry_[Util::EnumInt(pass_type)];
        ASSERT(reg);
        // Create a new UniformBlock, making sure to use the correct registry.
        block.reset(new UniformBlock(pass_type));
        GetContext()->registry_stack.push(reg);
        block->SetUpIon(GetContext());
        GetContext()->registry_stack.pop();
        uniform_blocks_.GetValue().push_back(block);
        ion_node_->AddUniformBlock(block->GetIonUniformBlock());
    }
    return block;
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

void Node::UpdateForRenderPass(PassType pass_type) {
    for (const auto &block: GetUniformBlocks())
        block->GetIonUniformBlock()->Enable(PassesMatch(block->GetPassType(),
                                                        pass_type));
}

void Node::SetUpIon(const ContextPtr &context) {
    Object::SetUpIon(context);

    if (! ion_node_) {
        ion_node_.Reset(new ion::gfx::Node);
        ion_node_->SetLabel(GetName());

        // Use the global registry for non-pass-specific uniform blocks.
        registry_[Util::EnumInt(PassType::kAnyPass)] =
            ion::gfx::ShaderInputRegistry::GetGlobalRegistry();

        if (auto &st = GetStateTable()) {
            st->SetUpIon(context);
            ion_node_->SetStateTable(st->GetIonStateTable());
        }
        if (auto &prog = GetShaderProgram()) {
            prog->SetUpIon(context);
            const auto &ion_prog = prog->GetIonShaderProgram();
            ion_node_->SetShaderProgram(ion_prog);
            // Push the registry on the stack.
            context->registry_stack.push(ion_prog->GetRegistry());
        }
        // Set up UniformBlocks after the shader, since they require the proper
        // registry to be in place.
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

        // Save the current registry in this Node. This is needed for creating
        // uniform blocks.
        registry_[Util::EnumInt(context->pass_type)] =
            context->registry_stack.top();

        // If there is no UniformBlock that is not pass-specific, create and
        // set one up. This is needed for matrix uniform handling.
        GetUniformBlockForPass(PassType::kAnyPass, true);

        // Check for changes to transform fields.
        if (scale_.WasSet() || rotation_.WasSet() ||
            translation_.WasSet()) {
            ProcessChange_(Change::kTransform);
        }

        // Restore the previous registry.
        if (GetShaderProgram()) {
            ASSERT(context->registry_stack.top() ==
                   GetShaderProgram()->GetIonShaderProgram()->GetRegistry());
            context->registry_stack.pop();
        }
    }
    else {
        // Make sure the registry stack is accurate.
        auto &prog = GetShaderProgram();
        if (prog)
            context->registry_stack.push(
                prog->GetIonShaderProgram()->GetRegistry());

        if (! registry_[Util::EnumInt(context->pass_type)])
            registry_[Util::EnumInt(context->pass_type)] =
                context->registry_stack.top();

        // Set up UniformBlocks again, since they may be pass-specific.
        for (const auto &block: GetUniformBlocks())
            block->SetUpIon(context);

        // Recurse on children again.
        for (const auto &child: GetChildren())
            child->SetUpIon(context);

        if (prog)
            context->registry_stack.pop();
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
        // Find the UniformBlock that is not pass-specific. It should have been
        // created in SetUpIon() if necessary.
        UniformBlockPtr block =
            GetUniformBlockForPass(PassType::kAnyPass, false);
        ASSERT(block);
        block->SetModelMatrices(matrix_, matrix_);
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
