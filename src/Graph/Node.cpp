#include "Graph/Node.h"

#include <ion/gfx/shaderinputregistry.h>
#include <ion/math/matrix.h>
#include <ion/math/transformutils.h>

#include "Graph/ShaderProgram.h"
#include "Graph/Shape.h"
#include "Graph/Texture.h"

namespace Graph {

Node::Node() : i_node_(new ion::gfx::Node) {
    // Always add a uModelviewMatrix uniform for transformation info.
    ion::gfx::ShaderInputRegistryPtr reg =
        ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
    matrix_index_ = AddUniform_(
        reg->Create<ion::gfx::Uniform>("uModelviewMatrix",
                                       ion::math::Matrix4f::Identity()));
}

void Node::SetName_(const std::string &name) {
    Object::SetName_(name);
    i_node_->SetLabel(name);
}

void Node::SetEnabled(bool enabled) {
    i_node_->Enable(enabled);
}

bool Node::IsEnabled() const {
    return i_node_->IsEnabled();
}

void Node::SetScale(const ion::math::Vector3f &scale) {
    scale_ = scale;
    UpdateMatrix_();
}

void Node::SetRotation(const ion::math::Rotationf &rotation) {
    rotation_ = rotation;
    UpdateMatrix_();
}

void Node::SetTranslation(const ion::math::Vector3f &translation) {
    translation_ = translation;
    UpdateMatrix_();
}

void Node::SetStateTable_(const ion::gfx::StateTablePtr &state_table) {
    i_node_->SetStateTable(state_table);
}

void Node::SetShaderProgram_(const ShaderProgramPtr &program) {
    shader_program_ = program;
    i_node_->SetShaderProgram(program->GetIonShaderProgram());
}

int Node::AddUniform_(const ion::gfx::Uniform &uniform) {
    return i_node_->AddUniform(uniform);
}

void Node::AddTexture_(const TexturePtr &texture) {
    textures_.push_back(texture);
}

void Node::AddShape_(const ShapePtr &shape) {
    i_node_->AddShape(shape->GetIonShape());
    shapes_.push_back(shape);
}

void Node::ClearChildren_() {
    i_node_->ClearChildren();
    children_.clear();
}

void Node::AddChild_(const NodePtr &child) {
    i_node_->AddChild(child->i_node_);
    children_.push_back(child);
}

void Node::UpdateMatrix_() {
    assert(matrix_index_ >= 0);
    i_node_->SetUniformValue(matrix_index_,
                             ion::math::TranslationMatrix(translation_) *
                             ion::math::RotationMatrixH(rotation_) *
                             ion::math::ScaleMatrixH(scale_));
}

}  // namespace Graph
