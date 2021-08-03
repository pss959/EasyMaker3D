#include "Scene.h"

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/shape.h>
#include <ion/gfx/uniform.h>
#include <ion/gfxutils/frame.h>
#include <ion/gfxutils/shadermanager.h>
#include <ion/gfxutils/shapeutils.h>
#include <ion/math/matrix.h>
#include <ion/math/range.h>
#include <ion/math/vector.h>

using ion::gfx::NodePtr;
using ion::gfx::StateTablePtr;
using ion::math::Point2i;
using ion::math::Point3f;
using ion::math::Range2i;
using ion::math::Vector2i;
using ion::math::Vector3f;
using ion::math::Vector4f;
using ion::math::Matrix4f;

// XXXX For testing!
static NodePtr BuildCyl_(const Point3f &pos, const Vector4f &color) {
    ion::gfxutils::CylinderSpec cyl_spec;
    cyl_spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
    cyl_spec.top_radius    = 1.f;
    cyl_spec.bottom_radius = 2.f;
    cyl_spec.height        = 3.f;
    cyl_spec.translation   = pos;

    NodePtr cyl_node(new ion::gfx::Node);
    const ion::gfx::ShaderInputRegistryPtr& global_reg =
        ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
    cyl_node->AddShape(ion::gfxutils::BuildCylinderShape(cyl_spec));
    cyl_node->AddUniform(
        global_reg->Create<ion::gfx::Uniform>("uBaseColor", color));

    return cyl_node;
}


Scene::Scene() {
    BuildStateTable_();
    BuildGraph_();
}

Scene::~Scene() {
}

void Scene::SetProjection(const Matrix4f &proj) {
    scene_root_->SetUniformValue(proj_index_, proj);
}

void Scene::SetView(const Matrix4f &view) {
    scene_root_->SetUniformValue(view_index_, view);
}

void Scene::SetViewport(const Range2i &viewport) {
    state_table_->SetViewport(viewport);
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
    const Matrix4f proj(1.732f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.732f, 0.0f, 0.0f,
                        0.0f, 0.0f, -1.905f, -13.798f,
                        0.0f, 0.0f, -1.0f, 0.0f);
    const Matrix4f view(1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, -10.0f,
                        0.0f, 0.0f, 0.0f, 1.0f);
    proj_index_ = scene_root_->AddUniform(
        global_reg->Create<ion::gfx::Uniform>("uProjectionMatrix", proj));
    view_index_ = scene_root_->AddUniform(
        global_reg->Create<ion::gfx::Uniform>("uModelviewMatrix", view));
    scene_root_->AddUniform(
        global_reg->Create<ion::gfx::Uniform>("uBaseColor",
                                              Vector4f(1.f, 1.f, 0.f, 1.f)));

    scene_root_->AddChild(BuildCyl_(Point3f( 4, 0, 0), Vector4f(1, 0, 0, 1)));
    scene_root_->AddChild(BuildCyl_(Point3f(-4, 0, 0), Vector4f(1, 0, 0, 1)));
    scene_root_->AddChild(BuildCyl_(Point3f(0,  4, 0), Vector4f(0, 1, 0, 1)));
    scene_root_->AddChild(BuildCyl_(Point3f(0, -4, 0), Vector4f(0, 1, 0, 1)));
    scene_root_->AddChild(BuildCyl_(Point3f(0, 0,  4), Vector4f(0, 0, 1, 1)));
    scene_root_->AddChild(BuildCyl_(Point3f(0, 0, -4), Vector4f(0, 0, 1, 1)));
}
