#include "SG/Node.h"

#include <ion/gfx/shaderinputregistry.h>
#include <ion/math/matrix.h>
#include <ion/math/transformutils.h>

#include "SG/ShaderProgram.h"
#include "SG/Shape.h"
#include "SG/SpecBuilder.h"
#include "SG/StateTable.h"
#include "SG/Texture.h"
#include "SG/Uniform.h"

namespace SG {

void Node::SetEnabled(bool enabled) {
    ion_node_->Enable(enabled);
}

bool Node::IsEnabled() const {
    return ion_node_->IsEnabled();
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

void Node::UpdateMatrix_() {
    assert(matrix_index_ >= 0);
    ion_node_->SetUniformValue(matrix_index_,
                               ion::math::TranslationMatrix(translation_) *
                               ion::math::RotationMatrixH(rotation_) *
                               ion::math::ScaleMatrixH(scale_));
}

NParser::ObjectSpec Node::GetObjectSpec() {
    SG::SpecBuilder<Node> builder;
    builder.AddVector3f("scale",                 &Node::scale_);
    builder.AddRotationf("rotation",             &Node::rotation_);
    builder.AddVector3f("translation",           &Node::translation_);
    builder.AddObject<StateTable>("state_table", &Node::state_table_);
    builder.AddObject<ShaderProgram>("shader",   &Node::shader_program_);
    builder.AddObjectList<Texture>("textures",   &Node::textures_);
    builder.AddObjectList<Uniform>("uniforms",   &Node::uniforms_);
    builder.AddObjectList<Shape>("shapes",       &Node::shapes_);
    builder.AddObjectList<Node>("children",      &Node::children_);
    return NParser::ObjectSpec{
        "Node", []{ return new Node; }, builder.GetSpecs() };
}

}  // namespace SG
