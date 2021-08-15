#include "Graph/Node.h"

#include "Graph/ShaderProgram.h"
#include "Graph/Shape.h"
#include "Graph/Texture.h"

namespace Graph {

Node::Node() : i_node_(new ion::gfx::Node) {
}

void Node::SetName_(const std::string &name) {
    Object::SetName_(name);
    i_node_->SetLabel(name);
}

void Node::SetEnabled_(bool enabled) {
    i_node_->Enable(enabled);
}

void Node::SetScale_(const ion::math::Vector3f &scale) {
    transform_.SetScale(scale);
}

void Node::SetRotation_(const ion::math::Rotationf &rotation) {
    transform_.SetRotation(rotation);
}

void Node::SetTranslation_(const ion::math::Vector3f &translation) {
    transform_.SetTranslation(translation);
}

void Node::SetStateTable_(const ion::gfx::StateTablePtr &state_table) {
    i_node_->SetStateTable(state_table);
}

void Node::SetShaderProgram_(const ShaderProgramPtr &program) {
    shader_program_ = program;
    i_node_->SetShaderProgram(program->GetIonShaderProgram());
}

void Node::AddUniform_(const ion::gfx::Uniform &uniform) {
    i_node_->AddUniform(uniform);
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

}  // namespace Graph
