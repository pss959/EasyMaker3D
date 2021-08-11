#include "Scene.h"

#include <iostream>

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/shape.h>
#include <ion/gfx/uniform.h>
#include <ion/gfxutils/frame.h>
#include <ion/gfxutils/printer.h>
#include <ion/gfxutils/shapeutils.h>
#include <ion/math/matrix.h>
#include <ion/math/range.h>
#include <ion/math/transformutils.h>
#include <ion/math/vector.h>

#include "Interfaces/IResourceManager.h"
#include "View.h"

using ion::gfx::NodePtr;
using ion::gfx::StateTablePtr;
using ion::math::Vector4f;
using ion::math::Matrix4f;

Scene::Scene(IResourceManager &resource_manager) :
    resource_manager_(resource_manager) {
    BuildStateTable_();
    Reload();
}

Scene::~Scene() {
}

void Scene::Reload() {
    BuildGraph_();
    scene_root_->AddChild(resource_manager_.LoadNode("test.mvn"));
}

void Scene::PrintScene() const {
    ion::gfxutils::Printer printer;
    printer.EnableAddressPrinting(false);
    printer.EnableFullShapePrinting(false);
    printer.SetFloatCleanTolerance(1e-5f);  // Clean values close to zero.
    printer.PrintScene(scene_root_, std::cout);
}

void Scene::UpdateFromView(const View &view) {
    scene_root_->SetUniformValue(proj_index_, view.projection_matrix);

    if (view.camera_rotation.IsIdentity()) {
        scene_root_->SetUniformValue(view_index_, view.view_matrix);
    }
    else {
        scene_root_->SetUniformValue(
            view_index_,
            view.view_matrix *
            ion::math::RotationMatrixH(view.camera_rotation));
    }

    state_table_->SetViewport(view.viewport_rect);
}

void Scene::BuildStateTable_() {
    state_table_.Reset(new ion::gfx::StateTable());
    state_table_->SetClearColor(Vector4f(0.3f, 0.3f, 0.5f, 1.0f));
    state_table_->SetClearDepthValue(1.f);
    state_table_->Enable(ion::gfx::StateTable::kDepthTest, true);
    state_table_->Enable(ion::gfx::StateTable::kCullFace,  true);
}

void Scene::BuildGraph_() {
    scene_root_.Reset(new ion::gfx::Node);
    scene_root_->SetStateTable(state_table_);

    const ion::gfx::ShaderInputRegistryPtr& global_reg =
        ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
    proj_index_ = scene_root_->AddUniform(
        global_reg->Create<ion::gfx::Uniform>("uProjectionMatrix",
                                              Matrix4f::Identity()));
    view_index_ = scene_root_->AddUniform(
        global_reg->Create<ion::gfx::Uniform>("uModelviewMatrix",
                                              Matrix4f::Identity()));
}
