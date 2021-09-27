#include "SG/Node.h"

#include "Math/Linear.h"
#include "Parser/Registry.h"
#include "SG/Exception.h"
#include "Util/KLog.h"

namespace SG {

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

bool Node::IsValid(std::string &details) {
    if (! Object::IsValid(details))
        return false;

    // Set up notification from shapes and child nodes.
    for (const auto &shape: GetShapes())
        AddAsShapeObserver_(*shape);
    for (const auto &child: GetChildren())
        AddAsChildNodeObserver_(*child);

    // Check for changes to transform fields.
    if (scale_.WasSet() || rotation_.WasSet() || translation_.WasSet()) {
        ProcessChange(Change::kTransform);
    }
    return true;
}

NodePtr Node::Create(const std::string &name) {
    NodePtr node = Parser::Registry::CreateObject<Node>("Node");
    node->ChangeName(name);
    return node;
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

int Node::GetChildIndex(const NodePtr &child) const {
    const auto &children = GetChildren();
    auto it = std::find(children.begin(), children.end(), child);
    return it != children.end() ? it - children.begin() : -1;
}

NodePtr Node::GetChild(size_t index) const {
    const auto &children = GetChildren();
    return index < children.size() ? children[index] : NodePtr();
}

void Node::AddChild(const NodePtr &child) {
    children_.Add(child);
    AddAsChildNodeObserver_(*child);
    ASSERT(children_.WasSet());
}

void Node::InsertChild(size_t index, const NodePtr &child) {
    ASSERT(child);
    if (index >= GetChildCount())
        children_.Add(child);
    else
        children_.Insert(index, child);
    AddAsChildNodeObserver_(*child);
    ASSERT(children_.WasSet());
}

void Node::RemoveChild(size_t index) {
    RemoveAsChildNodeObserver_(*GetChild(index));
    children_.Remove(index);
    ASSERT(children_.WasSet());
}

void Node::ReplaceChild(size_t index, const NodePtr &new_child) {
    RemoveAsChildNodeObserver_(*GetChild(index));
    children_.Replace(index, new_child);
    AddAsChildNodeObserver_(*new_child);
    ASSERT(children_.WasSet());
}

const Bounds & Node::GetBounds() {
    if (! bounds_valid_) {
        bounds_ = UpdateBounds();
        bounds_valid_ = true;
        KLOG('b', "Updated bounds for " << GetDesc() << " to " << bounds_);
    }
    return bounds_;
}

void Node::UpdateForRendering() {
    ASSERT(ion_node_);

    // Each of these updates if necessary.
    GetModelMatrix();
    GetBounds();

    // If the node is not enabled for traversal, just disable the Ion Node and
    // stop.
    if (! IsEnabled(SG::Node::Flag::kTraversal)) {
        ion_node_->Enable(false);
        return;
    }

    // Enable or disable all UniformBlocks.
    const bool render_enabled = IsEnabled(SG::Node::Flag::kRender);
    for (const auto &block: GetUniformBlocks()) {
        auto &ion_block = block->GetIonUniformBlock();
        ASSERT(ion_block);
        ion_block->Enable(render_enabled);
    }

    // Enable or disable shape rendering.
    EnableShapes_(render_enabled);
}

void Node::EnableShapes_(bool enabled) {
    // Ion Shapes cannot be enabled or disabled. To disable rendering shapes,
    // they are temporarily moved into the saved_shapes_ vector.
    if (enabled) {
        if (! saved_shapes_.empty()) {
            for (auto &shape: saved_shapes_)
                ion_node_->AddShape(shape);
            saved_shapes_.clear();
        }
    }
    else {
        for (auto &shape: ion_node_->GetShapes())
            saved_shapes_.push_back(shape);
        ion_node_->ClearShapes();
    }
}

Bounds Node::UpdateBounds() {
    // Collect and combine Bounds from all shapes and children.
    Bounds bounds;
    for (const auto &shape: GetShapes())
        bounds.ExtendByRange(shape->GetBounds());
    for (const auto &child: GetChildren())
        bounds.ExtendByRange(TransformBounds(child->GetBounds(),
                                             child->GetModelMatrix()));
    return bounds;
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
    UniformBlockPtr block =
        Parser::Registry::CreateObject<UniformBlock>("UniformBlock");
    // Set the name of the UniformBlock to restrict it to the named pass.
    block->ChangeName(pass_name);
    uniform_blocks_.Add(block);
    return block;
}

void Node::ProcessChange(const Change &change) {
    // Prevent crashes during destruction.
    if (IsBeingDestroyed())
        return;

    KLOG('n', GetDesc() << " got change " << Util::EnumName(change));

    // Any change except appearance should invalidate bounds.
    if (change != Change::kAppearance) {
        bounds_valid_ = false;
        KLOG('b', "Invalidated bounds for " << GetDesc());
    }
    if (change == Change::kTransform) {
        matrices_valid_ = false;
        KLOG('m', GetDesc() << " invalidated matrices");
    }

    // Pass notification to observers.
    changed_.Notify(change);
}

void Node::AddShape(const ShapePtr &shape) {
    ASSERT(shape);
    shapes_.Add(shape);
    AddAsShapeObserver_(*shape);
}

void Node::AddAsShapeObserver_(Shape &shape) {
    KLOG('n', GetDesc() << " observing " << shape.GetDesc());
    shape.GetChanged().AddObserver(
        this, std::bind(&Node::ProcessChange, this, std::placeholders::_1));
}

void Node::AddAsChildNodeObserver_(Node &child) {
    KLOG('n', GetDesc() << " observing child " << child.GetDesc());
    child.GetChanged().AddObserver(
        this, std::bind(&Node::ProcessChange, this, std::placeholders::_1));
}

void Node::RemoveAsChildNodeObserver_(Node &child) {
    KLOG('n', GetDesc() << " unobserving child " << child.GetDesc());
    child.GetChanged().RemoveObserver(this);
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

}  // namespace SG
