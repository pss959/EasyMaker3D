#include "gfx.h"

// Work around GL incompatibilities.
#undef GL_GLEXT_VERSION

#include <iostream>

#include "ion/gfx/node.h"
#include "ion/gfx/renderer.h"
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

#if ENABLE_ION_REMOTE
#include <ion/remote/nodegraphhandler.h>
#include <ion/remote/remoteserver.h>
#include <ion/remote/remoteserver.h>
#include <ion/remote/resourcehandler.h>
#include <ion/remote/settinghandler.h>
#include <ion/remote/shaderhandler.h>
#include "ion/remote/tracinghandler.h"
#endif

using ion::math::Point2i;
using ion::math::Point3f;
using ion::math::Range2i;
using ion::math::Vector2i;
using ion::math::Vector4f;
using ion::math::Matrix4f;

// ----------------------------------------------------------------------------
// Helper_ class definition.
// ----------------------------------------------------------------------------

class GFX::Helper_ {
  public:
    Helper_(int width, int height);
    ~Helper_();
    Display     * GetDisplay()  const;
    GLXContext    GetContext()  const;
    GLXDrawable   GetDrawable() const;
    void Draw();

  private:
    ion::gfx::RendererPtr           renderer_;
    ion::gfxutils::ShaderManagerPtr shader_manager_;
    ion::gfxutils::FramePtr         frame_;
    ion::gfx::NodePtr               scene_root_;

#if ENABLE_ION_REMOTE
    std::unique_ptr<ion::remote::RemoteServer> remote_;
#endif

    const ion::gfx::NodePtr BuildGraph(int width, int height);
    void SetUpRemoteServer();
};

// ----------------------------------------------------------------------------
// GFX class.
// ----------------------------------------------------------------------------

GFX::GFX(int width, int height) : helper_(new Helper_(width, height)) {
}

GFX::~GFX() {
}

Display     * GFX::GetDisplay()  const { return helper_->GetDisplay();  }
GLXContext    GFX::GetContext()  const { return helper_->GetContext();  }
GLXDrawable   GFX::GetDrawable() const { return helper_->GetDrawable(); }

void GFX::Draw() {
    helper_->Draw();
}

// ----------------------------------------------------------------------------
// Helper_ class functions.
// ----------------------------------------------------------------------------

#if GLDEBUG_ENABLED
// XXXX
static void GLMessageCallback(GLenum source, GLenum type,
                              GLuint id, GLenum severity,
                              GLsizei length, const GLchar* message,
                              const void* userParam) {
    std::cerr << "GL " << (type == GL_DEBUG_TYPE_ERROR ? "ERROR " : "MSG")
              << " type = " << type
              << " severity = " << severity
              << ": " << message << "\n";
}
#endif


GFX::Helper_::Helper_(int width, int height) {
    // Forces Ion to find GL Context for some reason. XXXX
    glXGetCurrentContext();

#if GLDEBUG_ENABLED
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GLMessageCallback, 0);  // XXXX
#endif

    ion::gfx::GraphicsManagerPtr manager(new ion::gfx::GraphicsManager);
    renderer_.Reset(new ion::gfx::Renderer(manager));
    shader_manager_.Reset(new ion::gfxutils::ShaderManager);
    frame_.Reset(new ion::gfxutils::Frame);
    scene_root_ = BuildGraph(width, height);
    SetUpRemoteServer();
}

GFX::Helper_::~Helper_() {
#if ENABLE_ION_REMOTE
    remote_.reset(nullptr);
#endif
    scene_root_.Reset(nullptr);
    renderer_.Reset(nullptr);
}

Display * GFX::Helper_::GetDisplay() const {
    return XOpenDisplay(nullptr);
}

GLXContext GFX::Helper_::GetContext() const {
    return glXGetCurrentContext();
}

GLXDrawable GFX::Helper_::GetDrawable() const {
    return glXGetCurrentDrawable();
}

void GFX::Helper_::Draw() {
    renderer_->DrawScene(scene_root_);
}

const ion::gfx::NodePtr GFX::Helper_::BuildGraph(int width, int height) {
    ion::gfx::NodePtr root(new ion::gfx::Node);
    ion::gfxutils::RectangleSpec rect_spec;
    rect_spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
    rect_spec.size.Set(2.f, 2.f);
    root->AddShape(ion::gfxutils::BuildRectangleShape(rect_spec));
    ion::gfx::StateTablePtr state_table(
        new ion::gfx::StateTable(width, height));
    state_table->SetViewport(
        ion::math::Range2i::BuildWithSize(ion::math::Point2i(0, 0),
                                          ion::math::Vector2i(width, height)));
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

void GFX::Helper_::SetUpRemoteServer() {
#if ENABLE_ION_REMOTE
#if XXXX_REMOTE_MESSES_UP_STEAMVR
    remote_.reset(new ion::remote::RemoteServer(1234));

    ion::remote::NodeGraphHandlerPtr ngh(new ion::remote::NodeGraphHandler);
    ngh->AddNode(scene_root_);
    remote_->RegisterHandler(ngh);
    remote_->RegisterHandler(
        ion::remote::HttpServer::RequestHandlerPtr(
            new ion::remote::ResourceHandler(renderer_)));
    remote_->RegisterHandler(
        ion::remote::HttpServer::RequestHandlerPtr(
            new ion::remote::SettingHandler()));
    remote_->RegisterHandler(
        ion::remote::HttpServer::RequestHandlerPtr(
            new ion::remote::ShaderHandler(shader_manager_, renderer_)));
    remote_->RegisterHandler(
        ion::remote::HttpServer::RequestHandlerPtr(
            new ion::remote::TracingHandler(frame_, renderer_)));
#endif
#endif
}
