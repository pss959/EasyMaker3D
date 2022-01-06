#include "SG/Node.h"

#include "Math/Linear.h"
#include "Parser/Registry.h"
#include "SG/Exception.h"
#include "Util/KLog.h"

namespace SG {

Node::~Node() {
    if (were_shapes_and_children_observed_)
        UnobserveShapesAndChildren_();
}

void Node::AddFields() {
    AddField(disabled_flags_);
    AddField(pass_name_);
    AddField(scale_);
    AddField(rotation_);
    AddField(translation_);
    AddField(shader_names_);
    AddField(use_bounds_proxy_);
    AddField(state_table_);
    AddField(uniform_blocks_);
    AddField(shapes_);
    AddField(children_);
    Object::AddFields();
}

void Node::AllFieldsParsed(bool is_template) {
    if (! is_template) {
        if (! IsClone())
            ObserveShapesAndChildren_();

        // Check for changes to transform fields.
        if (scale_.WasSet() || rotation_.WasSet() || translation_.WasSet())
            ProcessChange(Change::kTransform);
    }
}

NodePtr Node::Create(const std::string &name) {
    NodePtr node = Parser::Registry::CreateObject<Node>(name);
    // Make sure the object knows parsing is done.
    node->AllFieldsParsed(false);
    return node;
}

void Node::SetEnabled(Flag flag, bool b) {
    // Inverse setting, since flags indicate what is disabled.
    if (b)
        disabled_flags_.Reset(flag);
    else
        disabled_flags_.Set(flag);

    // This potentially changes graph structure.
    ProcessChange(Change::kGraph);
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

const Matrix4f & Node::GetModelMatrix() const {
    if (! matrices_valid_) {
        UpdateMatrices_();
        matrices_valid_ = true;
    }
    return matrix_;
}

void Node::SetBaseColor(const Color &color) {
    GetUniformBlockForPass("Lighting").SetBaseColor(color);
    ProcessChange(Change::kAppearance);
}

void Node::SetEmissiveColor(const Color &color) {
    GetUniformBlockForPass("Lighting").SetEmissiveColor(color);
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

std::vector<NodePtr> Node::GetAllChildren() const {
    std::vector<NodePtr> children = GetChildren();
    Util::AppendVector(extra_children_, children);
    return children;
}

void Node::AddChild(const NodePtr &child) {
    children_.Add(child);
    SetUpChild_(*child);
    ProcessChange(Change::kGraph);
}

void Node::InsertChild(size_t index, const NodePtr &child) {
    ASSERT(child);
    if (index >= GetChildCount())
        children_.Add(child);
    else
        children_.Insert(index, child);
    SetUpChild_(*child);
    ProcessChange(Change::kGraph);
}

void Node::RemoveChild(size_t index) {
    const NodePtr child = GetChild(index);
    if (ion_node_ && child->ion_node_)
        ion_node_->RemoveChild(child->ion_node_);
    Unobserve(*child);
    children_.Remove(index);
    ASSERT(children_.WasSet());
    ProcessChange(Change::kGraph);
}

void Node::RemoveChild(const NodePtr &child) {
    const int index = GetChildIndex(child);
    ASSERT(index >= 0);
    RemoveChild(index);
}

void Node::ReplaceChild(size_t index, const NodePtr &new_child) {
    const NodePtr child = GetChild(index);
    if (ion_node_ && child->ion_node_)
        ion_node_->RemoveChild(child->ion_node_);
    Unobserve(*child);
    children_.Replace(index, new_child);
    SetUpChild_(*child);
    ProcessChange(Change::kGraph);
}

void Node::AddShape(const ShapePtr &shape) {
    ASSERT(shape);
    shapes_.Add(shape);

    // Make sure the shape is set up.
    if (ion_node_)
        ion_node_->AddShape(shape->SetUpIon());

    Observe(*shape);
    ProcessChange(Change::kGraph);
}

const Bounds & Node::GetBounds() const {
    if (! bounds_valid_) {
        bounds_ = UpdateBounds();
        KLOG('b', "Updated bounds for " << GetDesc() << " to " << bounds_);
        bounds_valid_ = true;
    }
    return bounds_;
}

Bounds Node::GetScaledBounds() const {
    return ScaleBounds(GetBounds(), GetScale());
}

UniformBlock & Node::GetUniformBlockForPass(const std::string &pass_name) {
    const UniformBlockPtr block = FindUniformBlockForPass_(pass_name);
    return block ? *block : *AddUniformBlock_(pass_name);
}

ion::gfx::NodePtr Node::SetUpIon(
    const IonContextPtr &ion_context,
    const std::vector<ion::gfx::ShaderProgramPtr> &programs) {
    // This needs to be called only once.
    if (ion_node_)
        return ion_node_;

    KLOG('I', "SetUpIon called for " << GetDesc());

    PreSetUpIon();

    ion_node_.Reset(new ion::gfx::Node);
    ion_node_->SetLabel(GetName());

    ion_context_ = ion_context;
    programs_    = programs;

    // Update the programs stored in this Node if any shaders are specified.
    for (const auto &name: GetShaderNames()) {
        const auto &info = ion_context_->GetShaderProgramInfo(name);
        programs_[info.pass_index] = info.program;
    }

    // Set up StateTable.
    if (auto &state_table = GetStateTable())
        ion_node_->SetStateTable(state_table->SetUpIon());

    // Set up UniformBlocks.
    for (const auto &block: GetUniformBlocks()) {
        auto reg =
            ion_context_->GetRegistryForPass(block->GetPassName(), programs_);
        ion_node_->AddUniformBlock(block->SetUpIon(ion_context_, reg));
    }

    // Set up all Shapes.
    for (const auto &shape: GetShapes())
        ion_node_->AddShape(shape->SetUpIon());

    // Recurse on and add all children.
    for (const auto &child: GetAllChildren())
        ion_node_->AddChild(child->SetUpIon(ion_context, programs_));

    // Make sure the matrix and bounds are up to date and set in the Ion Node.
    matrices_valid_ = false;
    bounds_valid_   = false;
    GetModelMatrix();
    GetBounds();

    PostSetUpIon();

    return ion_node_;
}

void Node::UpdateForRenderPass(const std::string &pass_name) {
    ASSERTM(ion_node_, GetDesc() + " not set up for rendering");

    // Each of these updates if necessary.
    GetModelMatrix();
    GetBounds();

    // If the node is not enabled for traversal or it is pass-specific and
    // restricted to a different pass, just disable the Ion Node and stop.
    if (! IsEnabled(SG::Node::Flag::kTraversal) ||
        (! GetPassName().empty() && GetPassName() != pass_name)) {
        ion_node_->Enable(false);
        return;
    }
    ion_node_->Enable(true);

    // Enable or disable all UniformBlocks.
    const bool render_enabled = IsEnabled(SG::Node::Flag::kRender);
    for (const auto &block: GetUniformBlocks()) {
        const bool block_enabled = render_enabled &&
            (block->GetPassName().empty() || block->GetPassName() == pass_name);
        auto &ion_block = block->GetIonUniformBlock();
        ASSERT(ion_block);
        ion_block->Enable(block_enabled);
    }

    // Install the correct Ion ShaderProgram for the pass. There is no way to
    // disable an Ion ShaderProgram, so we have to set it to null to disable
    // it.
    if (! GetShaderNames().empty()) {
        // If the Node specifies one or more shader program names, see if the
        // name of the program for this pass is one of them. If so, install the
        // program. Otherwise, set it to null.
        const int index = ion_context_->GetPassIndex(pass_name);
        ion::gfx::ShaderProgramPtr program = programs_[index];
        if (! Util::Contains(GetShaderNames(), program->GetLabel()))
            program.Reset(nullptr);
        ion_node_->SetShaderProgram(program);
    }

    // Enable or disable shape rendering.
    EnableShapes_(render_enabled);
}

void Node::SetUpChild_(Node &child) {
    if (ion_node_)
        ion_node_->AddChild(child.SetUpIon(ion_context_, programs_));
    Observe(child);
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

void Node::CopyContentsFrom(const Parser::Object &from, bool is_deep) {
    Object::CopyContentsFrom(from, is_deep);
    if (! were_shapes_and_children_observed_)
        ObserveShapesAndChildren_();
}

Bounds Node::UpdateBounds() const {
    // Collect and combine Bounds from all shapes and children.
    Bounds bounds;
    for (const auto &shape: GetShapes())
        bounds.ExtendByRange(shape->GetBounds());
    for (const auto &child: GetAllChildren())
        bounds.ExtendByRange(TransformBounds(child->GetBounds(),
                                             child->GetModelMatrix()));
    return bounds;
}

void Node::ProcessChange(Change change) {
    // Prevent crashes during destruction.
    if (IsBeingDestroyed())
        return;

    // Any change except appearance should invalidate bounds.
    if (change != Change::kAppearance && bounds_valid_) {
        bounds_valid_ = false;
        KLOG('b', "Invalidated bounds for " << GetDesc());
    }
    if (change == Change::kTransform && matrices_valid_) {
        matrices_valid_ = false;
        KLOG('m', GetDesc() << " invalidated matrices");
    }

    Object::ProcessChange(change);
}

void Node::ClearExtraChildren() {
    for (auto &child: extra_children_) {
        if (ion_node_ && child->ion_node_)
            ion_node_->RemoveChild(child->ion_node_);
        Unobserve(*child);
        extra_children_.clear();
    }
    ProcessChange(Change::kGraph);
}

void Node::AddExtraChild(const NodePtr &child) {
    children_.Add(child);
    SetUpChild_(*child);
    ProcessChange(Change::kGraph);
}

void Node::UpdateMatrices_() const {
    matrix_ = GetTransformMatrix(GetScale(), GetRotation(), GetTranslation());
    KLOG('m', GetDesc() << " updated matrix in node");

    if (ion_node_) {
        // Special case: If there is no existing UniformBlock for the matrix
        // uniforms and the matrix is identity, there is no need to create one,
        // since the default is identity.
        if (matrix_ == Matrix4f::Identity() && ! FindUniformBlockForPass_(""))
            return;

        // Set up a UniformBlock to store the matrices if not already done. It
        // should use the global registry.
        Node *n = const_cast<Node *>(this);   // XXXX Ugly...
        n->GetUniformBlockForPass("").SetModelMatrices(matrix_, matrix_);
        KLOG('m', GetDesc() << " updated matrix in uniforms");
    }
}

UniformBlockPtr Node::FindUniformBlockForPass_(
    const std::string &pass_name) const {
    for (auto &block: GetUniformBlocks()) {
        if (block->GetPassName() == pass_name)
            return block;
    }
    return UniformBlockPtr();
}

UniformBlockPtr Node::AddUniformBlock_(const std::string &pass_name) {
    // Restrict the UniformBlock to the named pass.
    UniformBlockPtr block = Parser::Registry::CreateObject<UniformBlock>();
    block->SetPassName(pass_name);
    uniform_blocks_.Add(block);

    ASSERTM(ion_context_, "Missing context in " + GetDesc());
    ASSERT(ion_node_);
    auto reg = ion_context_->GetRegistryForPass(pass_name, programs_);
    ion_node_->AddUniformBlock(block->SetUpIon(ion_context_, reg));

    return block;
}

void Node::ObserveShapesAndChildren_() {
    ASSERT(! were_shapes_and_children_observed_);
    for (const auto &shape: GetShapes())
        Observe(*shape);
    for (const auto &child: GetAllChildren())
        Observe(*child);
    were_shapes_and_children_observed_ = true;
}

void Node::UnobserveShapesAndChildren_() {
    ASSERT(were_shapes_and_children_observed_);
    for (const auto &shape: GetShapes())
        Unobserve(*shape);
    for (const auto &child: GetAllChildren())
        Unobserve(*child);
    were_shapes_and_children_observed_ = false;
}

}  // namespace SG
