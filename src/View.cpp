#include "View.h"

#include <assert.h>

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/uniform.h>
#include <ion/gfxutils/printer.h>
#include <ion/math/rotation.h>
#include <ion/math/transformutils.h>
#include <ion/math/vector.h>

#include "Graph/Node.h"
#include "Graph/Scene.h"

using ion::gfx::NodePtr;
using ion::math::Vector4f;
using ion::math::Matrix4f;
using ion::math::Range2i;

View::View() : root_(BuildGraph_()) {
}

View::~View() {
}

void View::SetScene(const Graph::ScenePtr &scene) {
    assert(scene);
    assert(scene->GetRootNode());

    scene_ = scene;
    UpdateFromCamera(scene_->GetCamera());

    // Add the root of the Scene.
    root_->ClearChildren();
    root_->AddChild(scene_->GetRootNode()->GetIonNode());
}

void View::UpdateViewport(const Range2i &viewport_rect) {
    root_->GetStateTable()->SetViewport(viewport_rect);
}

void View::UpdateFromCamera(const Graph::Camera &camera) {
    root_->SetUniformValue(proj_index_, ComputeProjectionMatrix_(camera));
    root_->SetUniformValue(view_index_, ComputeViewMatrix_(camera));
}

void View::PrintContents() const {
    ion::gfxutils::Printer printer;
    printer.EnableAddressPrinting(false);
    printer.EnableFullShapePrinting(false);
    printer.SetFloatCleanTolerance(1e-5f);  // Clean values close to zero.
    printer.PrintScene(root_, std::cout);
}

NodePtr View::BuildGraph_() {
    NodePtr root(new ion::gfx::Node);

    // Set up the StateTable with reasonable defaults.
    ion::gfx::StateTablePtr table(new ion::gfx::StateTable());
    table->SetClearColor(Vector4f(0.3f, 0.3f, 0.5f, 1.0f));
    table->SetClearDepthValue(1.f);
    table->Enable(ion::gfx::StateTable::kDepthTest, true);
    table->Enable(ion::gfx::StateTable::kCullFace,  true);
    root->SetStateTable(table);

    // Add proj/view matrix uniforms and save their indices.
    const auto& reg = ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
    Matrix4f ident = Matrix4f::Identity();
    proj_index_ = root->AddUniform(
        reg->Create<ion::gfx::Uniform>("uProjectionMatrix", ident));
    view_index_ = root->AddUniform(
        reg->Create<ion::gfx::Uniform>("uModelviewMatrix", ident));

    return root;
}

Matrix4f View::ComputeProjectionMatrix_(const Graph::Camera &camera) {
    const float tan_l = tanf(camera.fov.left.Radians());
    const float tan_r = tanf(camera.fov.right.Radians());
    const float tan_u = tanf(camera.fov.up.Radians());
    const float tan_d = tanf(camera.fov.down.Radians());

    const float tan_lr = tan_r - tan_l;
    const float tan_du = tan_u - tan_d;

    const float near = camera.near;
    const float far  = camera.far;
    return Matrix4f(
        2 / tan_lr, 0, (tan_r + tan_l) / tan_lr, 0,
        0, 2 / tan_du, (tan_u + tan_d) / tan_du, 0,
        0, 0, -(far + near) / (far - near), -(2 * far * near) / (far - near),
        0, 0, -1, 0);
}

Matrix4f View::ComputeViewMatrix_(const Graph::Camera &camera) {
    return ion::math::RotationMatrixH(-camera.orientation) *
        ion::math::TranslationMatrix(-camera.position);
}
