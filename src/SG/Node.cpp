#include "SG/Node.h"

#include "Math/Linear.h"
#include "SG/Exception.h"
#include "Util/KLog.h"

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
    AddField(pass_data_);
    AddField(shapes_);
    AddField(children_);
}

void Node::AllFieldsParsed() {
    Object::AllFieldsParsed();
    // Set up notification from shapes and child nodes.
    for (const auto &shape: GetShapes()) {
        KLOG('n', GetDesc() << ") observing " << shape->GetDesc());
        shape->GetChanged().AddObserver(
            std::bind(&Node::ProcessChange_, this, std::placeholders::_1));
    }
    for (const auto &child: GetChildren()) {
        KLOG('n', GetDesc() << ") observing child " << child->GetDesc());
        child->GetChanged().AddObserver(
            std::bind(&Node::ProcessChange_, this, std::placeholders::_1));
    }

    // Check for changes to transform fields.
    if (scale_.WasSet() || rotation_.WasSet() || translation_.WasSet()) {
        ProcessChange_(Change::kTransform);
    }
}

void Node::CreateIonNode() {
    ASSERT(! ion_node_);
    ion_node_.Reset(new ion::gfx::Node);
    ion_node_->SetLabel(GetName());
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
    GetUniformBlockForPass_("Lighting").SetBaseColor(color);
    ProcessChange_(Change::kAppearance);
}

void Node::SetEmissiveColor(const Color &color) {
    GetUniformBlockForPass_("Lighting").SetEmissiveColor(color);
    ProcessChange_(Change::kAppearance);
}

const Bounds & Node::GetBounds() {
    if (! bounds_valid_) {
        UpdateBounds_();
        bounds_valid_ = true;
    }
    return bounds_;
}

UniformBlock & Node::GetUniformBlockForPass_(const std::string &pass_name) {
    for (auto &pass_data: GetPassData()) {
        // This has to be an exact match.
        if (pass_data->GetName() == pass_name) {
            ASSERT(pass_data->GetUniformBlock());
            return *pass_data->GetUniformBlock();
        }
    }
    throw Exception("No UniformBlock for pass " + pass_name +
                    " in " + GetDesc());
    return *UniformBlockPtr();
}

void Node::ProcessChange_(const Change &change) {
    // Prevent crashes during destruction.
    if (IsBeingDestroyed())
        return;

    KLOG('n', "Node " << this << " (" << GetName()
         << ") Got change " << Util::EnumName(change));

    // Any change except appearance should invalidate bounds.
    if (change != Change::kAppearance)
        bounds_valid_ = false;
    if (change == Change::kTransform)
        matrices_valid_ = false;

    // Pass notification to observers.
    changed_.Notify(change);
}

void Node::UpdateMatrices_() {
    ASSERT(ion_node_);

    matrix_ = GetTransformMatrix(GetScale(), GetRotation(), GetTranslation());

    // Set up a UniformBlock to store the matrices if not already done. It
    // should use the global registry.
    if (! matrix_uniform_block_) {
        matrix_uniform_block_.reset(new UniformBlock());
        matrix_uniform_block_->CreateIonUniformBlock();
        ion_node_->AddUniformBlock(matrix_uniform_block_->GetIonUniformBlock());
    }
    matrix_uniform_block_->SetModelMatrices(matrix_, matrix_);
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
