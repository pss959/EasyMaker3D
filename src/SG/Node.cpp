#include "SG/Node.h"

#include <ion/gfx/shaderinputregistry.h>
#include <ion/math/matrix.h>
#include <ion/math/matrixutils.h>
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
    need_to_update_matrices_ = true;
}

void Node::SetRotation(const ion::math::Rotationf &rotation) {
    rotation_ = rotation;
    need_to_update_matrices_ = true;
}

void Node::SetTranslation(const ion::math::Vector3f &translation) {
    translation_ = translation;
    need_to_update_matrices_ = true;
}

void Node::Update() {
    if (need_to_update_matrices_)
        UpdateMatrices_();
}

void Node::SetUpIon(IonContext &context) {
    if (! ion_node_) {
        ion_node_.Reset(new ion::gfx::Node);
        ion_node_->SetLabel(GetName());

        // Set a matrix from transform fields if any changed.
        if (scale_ != Vector3f(1, 1, 1) ||
            ! rotation_.IsIdentity() ||
            translation_ != Vector3f(0, 0, 0))
            need_to_update_matrices_ = true;

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
            AddTextureUniform_(context, *tex);
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

void Node::AddTextureUniform_(IonContext &context, const Texture &tex) {
    auto              &reg   = context.registry_stack.top();
    const std::string &name  = tex.GetUniformName();
    const int          count = tex.GetCount();

    ion::gfx::Uniform u;
    if (count > 1) {
        std::vector<ion::gfx::TexturePtr> texvec(count, tex.GetIonTexture());
        u = reg->CreateArrayUniform(name, texvec.data(), count,
                                    ion::base::AllocatorPtr());
    }
    else {
        u = reg->Create<ion::gfx::Uniform>(name, tex.GetIonTexture());
    }
    ion_node_->AddUniform(u);
}

void Node::UpdateMatrices_() {
    // Don't do this before SetUpIon() is called.
    ASSERT(ion_node_);

    const Matrix4f mm =
        ion::math::TranslationMatrix(translation_) *
        ion::math::RotationMatrixH(rotation_) *
        ion::math::ScaleMatrixH(scale_);
    const Matrix3f nm = ion::math::Transpose(
        ion::math::Inverse(ion::math::WithoutDimension(mm, 3)));

    // Create the uniforms if not already done.
    if (m_matrix_index_ < 0) {
        auto reg = ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
        m_matrix_index_ = ion_node_->AddUniform(
            reg->Create<ion::gfx::Uniform>("uModelviewMatrix", mm));
        n_matrix_index_ = ion_node_->AddUniform(
            reg->Create<ion::gfx::Uniform>("uNormalMatrix", nm));
    }
    else {
        ion_node_->SetUniformValue(m_matrix_index_, mm);
        ion_node_->SetUniformValue(n_matrix_index_, nm);
    }

    need_to_update_matrices_ = false;
}

}  // namespace SG
