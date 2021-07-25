#include "gfx.h"

#include <iostream>

#include "ion/gfx/node.h"
#include "ion/gfx/renderer.h"
#include "ion/gfx/shaderinputregistry.h"
#include "ion/gfx/shape.h"
#include "ion/gfx/statetable.h"
#include "ion/gfx/uniform.h"
#include "ion/gfxutils/shapeutils.h"
#include "ion/math/matrix.h"
#include "ion/math/range.h"
#include "ion/math/vector.h"

#include <GL/glx.h>

using ion::math::Point2i;
using ion::math::Point3f;
using ion::math::Range2i;
using ion::math::Vector2i;
using ion::math::Vector4f;
using ion::math::Matrix4f;

namespace gfx {

static ion::gfx::RendererPtr s_renderer;
static ion::gfx::NodePtr     s_scene_root;

static const ion::gfx::NodePtr BuildGraph(int window_width, int window_height) {
    ion::gfx::NodePtr root(new ion::gfx::Node);
    ion::gfxutils::RectangleSpec rect_spec;
    rect_spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
    rect_spec.size.Set(2.f, 2.f);
    root->AddShape(ion::gfxutils::BuildRectangleShape(rect_spec));
    ion::gfx::StateTablePtr state_table(
        new ion::gfx::StateTable(window_width, window_height));
    state_table->SetViewport(
        ion::math::Range2i::BuildWithSize(ion::math::Point2i(0, 0),
                                          ion::math::Vector2i(window_width,
                                                              window_height)));
    state_table->SetClearColor(ion::math::Vector4f(0.3f, 0.3f, 0.5f, 1.0f));
    state_table->SetClearDepthValue(1.f);
    state_table->Enable(ion::gfx::StateTable::kDepthTest, true);
    state_table->Enable(ion::gfx::StateTable::kCullFace, true);
    root->SetStateTable(state_table);
    const ion::gfx::ShaderInputRegistryPtr& global_reg =
        ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
    const ion::math::Matrix4f proj(1.732f, 0.0f, 0.0f, 0.0f,
                                   0.0f, 1.732f, 0.0f, 0.0f,
                                   0.0f, 0.0f, -1.905f, -13.798f,
                                   0.0f, 0.0f, -1.0f, 0.0f);
    const ion::math::Matrix4f view(1.0f, 0.0f, 0.0f, 0.0f,
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, -5.0f,
                                   0.0f, 0.0f, 0.0f, 1.0f);
    root->AddUniform(global_reg->Create<ion::gfx::Uniform>(
                         "uProjectionMatrix", proj));
    root->AddUniform(global_reg->Create<ion::gfx::Uniform>(
                         "uModelviewMatrix", view));
    root->AddUniform(global_reg->Create<ion::gfx::Uniform>(
                         "uBaseColor", ion::math::Vector4f(1.f, 1.f, 0.f, 1.f)));
    return root;
}

void Init(int window_width, int window_height) {
    // Forces Ion to find GL Context for some reason.
    glXGetCurrentContext();

    ion::gfx::GraphicsManagerPtr manager(new ion::gfx::GraphicsManager);
    s_renderer.Reset(new ion::gfx::Renderer(manager));
    s_scene_root = BuildGraph(window_width, window_height);
}

void Draw() {
    s_renderer->DrawScene(s_scene_root);
}

void CleanUp() {
    s_scene_root.Reset(nullptr);
    s_renderer.Reset(nullptr);
}

}  // namespace gfx
