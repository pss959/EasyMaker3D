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
    AddField(uniform_blocks_);
    AddField(shapes_);
    AddField(children_);
}

void Node::AllFieldsParsed() {
    Object::AllFieldsParsed();
    // Set up notification from shapes and child nodes.
    for (const auto &shape: GetShapes()) {
        KLOG('n', GetDesc() << " observing " << shape->GetDesc());
        shape->GetChanged().AddObserver(
            std::bind(&Node::ProcessChange, this, std::placeholders::_1));
    }
    for (const auto &child: GetChildren()) {
        KLOG('n', GetDesc() << " observing child " << child->GetDesc());
        child->GetChanged().AddObserver(
            std::bind(&Node::ProcessChange, this, std::placeholders::_1));
    }

    // Check for changes to transform fields.
    if (scale_.WasSet() || rotation_.WasSet() || translation_.WasSet()) {
        ProcessChange(Change::kTransform);
    }
}

void Node::CreateIonNode() {
    ASSERT(! ion_node_);
    ion_node_.Reset(new ion::gfx::Node);
    ion_node_->SetLabel(GetName());
}

void Node::SetScale(const ion::math::Vector3f &scale) {
    scale_ = scale;
    ProcessChange(Change::kTransform);
}

void Node::SetRotation(const ion::math::Rotationf &rotation) {
    rotation_ = rotation;
    ProcessChange(Change::kTransform);
}

void Node::SetTranslation(const ion::math::Vector3f &translation) {
    translation_ = translation;
    ProcessChange(Change::kTransform);
}

const Matrix4f & Node::GetModelMatrix() {
    if (! matrices_valid_) {
        UpdateMatrices_();
        matrices_valid_ = true;
    }
    return matrix_;
}

void Node::SetBaseColor(const Color &color) {
    GetUniformBlockForPass("Lighting", true)->SetBaseColor(color);
    ProcessChange(Change::kAppearance);
}

void Node::SetEmissiveColor(const Color &color) {
    GetUniformBlockForPass("Lighting", true)->SetEmissiveColor(color);
    ProcessChange(Change::kAppearance);
}

const Bounds & Node::GetBounds() {
    if (! bounds_valid_) {
        UpdateBounds_();
        bounds_valid_ = true;
    }
    return bounds_;
}

void Node::UpdateForRendering() {
    // Each of these updates if necessary.
    GetModelMatrix();
    GetBounds();
}

UniformBlockPtr Node::GetUniformBlockForPass(const std::string &pass_name,
                                             bool must_exist) {
    for (auto &block: GetUniformBlocks()) {
        if (block->GetName() == pass_name)
            return block;
    }
    if (must_exist)
        throw Exception("No UniformBlock for pass " + pass_name +
                        " in " + GetDesc());
    return UniformBlockPtr();
}

UniformBlockPtr Node::AddUniformBlock(const std::string &pass_name) {
    UniformBlockPtr block(new UniformBlock(pass_name));
    uniform_blocks_.GetValue().push_back(block);
    return block;
}

void Node::ProcessChange(const Change &change) {
    // Prevent crashes during destruction.
    if (IsBeingDestroyed())
        return;

    KLOG('n', GetDesc() << " got change " << Util::EnumName(change));

    // Any change except appearance should invalidate bounds.
    if (change != Change::kAppearance)
        bounds_valid_ = false;
    if (change == Change::kTransform) {
        matrices_valid_ = false;
        KLOG('m', GetDesc() << " invalidated matrices");
    }

    // Pass notification to observers.
    changed_.Notify(change);
}

void Node::UpdateMatrices_() {
    matrix_ = GetTransformMatrix(GetScale(), GetRotation(), GetTranslation());
    KLOG('m', GetDesc() << " updated matrix in node");

    if (ion_node_) {
        // Set up a UniformBlock to store the matrices if not already done. It
        // should use the global registry.
        UniformBlockPtr block = GetUniformBlockForPass("", false);
        if (! block) {
            block = AddUniformBlock("");
            block->CreateIonUniformBlock();
            ion_node_->AddUniformBlock(block->GetIonUniformBlock());
        }
        block->SetModelMatrices(matrix_, matrix_);
        KLOG('m', GetDesc() << " updated matrix in uniforms");
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
