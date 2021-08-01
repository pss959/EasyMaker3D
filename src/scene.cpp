#include "scene.h"

#include <iostream>

#include "ion/gfx/node.h"
#include "ion/gfx/shaderinputregistry.h"
#include "ion/gfx/shape.h"
#include "ion/gfx/statetable.h"
#include "ion/gfx/uniform.h"
#include "ion/gfxutils/frame.h"
#include "ion/gfxutils/shadermanager.h"
#include "ion/gfxutils/shapeutils.h"
#include "ion/math/matrix.h"
#include "ion/math/range.h"
#include "ion/math/vector.h"

using ion::gfx::NodePtr;
using ion::gfx::StateTablePtr;
using ion::math::Point2i;
using ion::math::Point3f;
using ion::math::Range2i;
using ion::math::Vector2i;
using ion::math::Vector3f;
using ion::math::Vector4f;
using ion::math::Matrix4f;

// ----------------------------------------------------------------------------
// Helper_ class definition.
// ----------------------------------------------------------------------------

class Scene::Helper_ {
  public:
    Helper_();

    void            SetProjection(const Matrix4f &proj);
    void            SetView(const Matrix4f &view);
    void            SetViewport(const Range2i &viewport);
    const NodePtr & GetRoot() const { return scene_root_; }

  private:
    StateTablePtr state_table_;
    NodePtr       scene_root_;
    size_t        proj_index_;
    size_t        view_index_;

    const StateTablePtr BuildStateTable_();
    const NodePtr       BuildGraph_(const StateTablePtr &state_table);
    void                AddCylinder_(const NodePtr &parent,
                                     const Point3f &pos,
                                     const Vector4f &color);
};

// ----------------------------------------------------------------------------
// Scene class.
// ----------------------------------------------------------------------------

Scene::Scene() : helper_(new Helper_()) {
}

Scene::~Scene() {
}

void Scene::SetProjection(const Matrix4f &proj) {
    helper_->SetProjection(proj);
}

void Scene::SetView(const Matrix4f &view) {
    helper_->SetView(view);
}

void Scene::SetViewport(const Range2i &viewport) {
    helper_->SetViewport(viewport);
}

const NodePtr & Scene::GetRoot() const {
    return helper_->GetRoot();
}

// ----------------------------------------------------------------------------
// Helper_ class functions.
// ----------------------------------------------------------------------------

Scene::Helper_::Helper_() {
    state_table_ = BuildStateTable_();
    scene_root_  = BuildGraph_(state_table_);
}

void Scene::Helper_::SetProjection(const Matrix4f &proj) {
    scene_root_->SetUniformValue(proj_index_, proj);
}

void Scene::Helper_::SetView(const Matrix4f &view) {
    scene_root_->SetUniformValue(view_index_, view);
}

void Scene::Helper_::SetViewport(const Range2i &viewport) {
    state_table_->SetViewport(viewport);
}

const StateTablePtr Scene::Helper_::BuildStateTable_() {
    StateTablePtr state_table(new ion::gfx::StateTable());
    state_table->SetClearColor(Vector4f(0.3f, 0.3f, 0.5f, 1.0f));
    state_table->SetClearDepthValue(1.f);
    state_table->Enable(ion::gfx::StateTable::kDepthTest, true);
    state_table->Enable(ion::gfx::StateTable::kCullFace,  true);
    return state_table;
}

const NodePtr Scene::Helper_::BuildGraph_(const StateTablePtr &state_table) {
    NodePtr root(new ion::gfx::Node);

    root->SetStateTable(state_table);

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
    proj_index_ = root->AddUniform(
        global_reg->Create<ion::gfx::Uniform>("uProjectionMatrix", proj));
    view_index_ = root->AddUniform(
        global_reg->Create<ion::gfx::Uniform>("uModelviewMatrix", view));
    root->AddUniform(
        global_reg->Create<ion::gfx::Uniform>("uBaseColor",
                                              Vector4f(1.f, 1.f, 0.f, 1.f)));

    AddCylinder_(root, Point3f( 4, 0, 0), Vector4f(1, 0, 0, 1));
    AddCylinder_(root, Point3f(-4, 0, 0), Vector4f(1, 0, 0, 1));
    AddCylinder_(root, Point3f(0,  4, 0), Vector4f(0, 1, 0, 1));
    AddCylinder_(root, Point3f(0, -4, 0), Vector4f(0, 1, 0, 1));
    AddCylinder_(root, Point3f(0, 0,  4), Vector4f(0, 0, 1, 1));
    AddCylinder_(root, Point3f(0, 0, -4), Vector4f(0, 0, 1, 1));

    return root;
}

void Scene::Helper_::AddCylinder_(const NodePtr &parent,
                                  const Point3f &pos, const Vector4f &color) {
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

    parent->AddChild(cyl_node);
}
