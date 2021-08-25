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

void Node::SetUpIon(IonContext &context) {
    if (! ion_node_) {
        ion_node_.Reset(new ion::gfx::Node);
        ion_node_->SetLabel(GetName());

        // Set a matrix from transform fields if any changed.
        if (scale_ != Vector3f(1, 1, 1) ||
            ! rotation_.IsIdentity() ||
            translation_ != Vector3f(0, 0, 0))
            UpdateMatrix_();

        if (state_table_) {
            state_table_->SetUpIon(context);
            ion_node_->SetStateTable(state_table_->GetIonStateTable());
        }
        if (shader_program_) {
            shader_program_->SetUpIon(context);
            const auto &ion_prog = shader_program_->GetIonShaderProgram();
            ion_node_->SetShaderProgram(ion_prog);
            // Push the registry on the stack.
            context.registry_stack.push(ion_prog->GetRegistry());
        }
        for (const auto &tex: textures_) {
            tex->SetUpIon(context);
            ion_node_->AddUniform(
                context.registry_stack.top()->Create<ion::gfx::Uniform>(
                    tex->GetUniformName(), tex->GetIonTexture()));
        }
        for (const auto &uni: uniforms_) {
            uni->SetUpIon(context);
            ion_node_->AddUniform(uni->GetIonUniform());
        }
        for (const auto &shape: shapes_) {
            shape->SetUpIon(context);
            ion_node_->AddShape(shape->GetIonShape());
        }
        for (const auto &child: children_) {
            child->SetUpIon(context);
            ion_node_->AddChild(child->GetIonNode());
        }

        // Restore the previous registry.
        if (shader_program_) {
            ASSERT(context.registry_stack.top() ==
                   shader_program_->GetIonShaderProgram()->GetRegistry());
            context.registry_stack.pop();
        }
    }
}

Parser::ObjectSpec Node::GetObjectSpec() {
    SG::SpecBuilder<Node> builder;
    builder.AddFlags<Flag>("disabled_flags",     &Node::disabled_flags_);
    builder.AddVector3f("scale",                 &Node::scale_);
    builder.AddRotationf("rotation",             &Node::rotation_);
    builder.AddVector3f("translation",           &Node::translation_);
    builder.AddObject<StateTable>("state_table", &Node::state_table_);
    builder.AddObject<ShaderProgram>("shader",   &Node::shader_program_);
    builder.AddObjectList<Texture>("textures",   &Node::textures_);
    builder.AddObjectList<Uniform>("uniforms",   &Node::uniforms_);
    builder.AddObjectList<Shape>("shapes",       &Node::shapes_);
    builder.AddObjectList<Node>("children",      &Node::children_);
    return Parser::ObjectSpec{
        "Node", false, []{ return new Node; }, builder.GetSpecs() };
}

void Node::UpdateMatrix_() {
    const Matrix4f m =
        ion::math::TranslationMatrix(translation_) *
        ion::math::RotationMatrixH(rotation_) *
        ion::math::ScaleMatrixH(scale_);

    // Create the uModelviewMatrix uniform if not already done.
    if (matrix_index_ < 0) {
        ion::gfx::ShaderInputRegistryPtr reg =
            ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
        matrix_index_ = ion_node_->AddUniform(
            reg->Create<ion::gfx::Uniform>("uModelviewMatrix", m));
    }
    else {
        ion_node_->SetUniformValue(matrix_index_, m);
    }
}

}  // namespace SG
