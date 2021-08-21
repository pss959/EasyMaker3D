#include "View.h"

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/uniform.h>
#include <ion/math/rotation.h>
#include <ion/math/transformutils.h>
#include <ion/math/vector.h>

#include "Frustum.h"

using ion::gfx::NodePtr;
using ion::math::Vector4f;
using ion::math::Matrix4f;

View::View() {
    BuildGraph_();
}

View::~View() {
}

void View::ClearNodes() {
    root_->ClearChildren();
}

void View::AddNode(const NodePtr &node) {
    root_->AddChild(node);
}

void View::SetViewport(const Viewport &viewport) {
    viewport_ = viewport;
    root_->GetStateTable()->SetViewport(viewport_);
}

void View::SetFrustum(const Frustum &frustum) {
    frustum_ = frustum;
    root_->SetUniformValue(proj_index_, ComputeProjectionMatrix_(frustum_));
    root_->SetUniformValue(view_index_, ComputeViewMatrix_(frustum_));
    root_->SetUniformValue(viewport_index_, viewport_.GetSize());
}

float View::GetAspectRatio() const {
    const auto &size = viewport_.GetSize();
    return static_cast<float>(size[0]) / size[1];
}

void View::BuildGraph_() {
    root_.Reset(new ion::gfx::Node);

    // Set up the StateTable with reasonable defaults.
    ion::gfx::StateTablePtr table(new ion::gfx::StateTable());
    table->SetClearColor(Vector4f(0.3f, 0.3f, 0.5f, 1.0f));
    table->SetClearDepthValue(1.f);
    table->Enable(ion::gfx::StateTable::kDepthTest, true);
    table->Enable(ion::gfx::StateTable::kCullFace,  true);
    root_->SetStateTable(table);

    // Add matrix and viewport uniforms and save their indices.
    const auto& reg = ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
    Matrix4f ident = Matrix4f::Identity();
    proj_index_ = root_->AddUniform(
        reg->Create<ion::gfx::Uniform>("uProjectionMatrix", ident));
    view_index_ = root_->AddUniform(
        reg->Create<ion::gfx::Uniform>("uModelviewMatrix", ident));
    viewport_index_ = root_->AddUniform(
        reg->Create<ion::gfx::Uniform>("uViewportSize", viewport_.GetSize()));
}

Matrix4f View::ComputeProjectionMatrix_(const Frustum &frustum) {
    const float tan_l = tanf(frustum.fov_left.Radians());
    const float tan_r = tanf(frustum.fov_right.Radians());
    const float tan_u = tanf(frustum.fov_up.Radians());
    const float tan_d = tanf(frustum.fov_down.Radians());

    const float tan_lr = tan_r - tan_l;
    const float tan_du = tan_u - tan_d;

    const float near = frustum.near;
    const float far  = frustum.far;
    return Matrix4f(
        2 / tan_lr, 0, (tan_r + tan_l) / tan_lr, 0,
        0, 2 / tan_du, (tan_u + tan_d) / tan_du, 0,
        0, 0, -(far + near) / (far - near), -(2 * far * near) / (far - near),
        0, 0, -1, 0);
}

Matrix4f View::ComputeViewMatrix_(const Frustum &frustum) {
    return ion::math::RotationMatrixH(-frustum.orientation) *
        ion::math::TranslationMatrix(-frustum.position);
}
