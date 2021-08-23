#include "Renderer.h"

#include <iostream>

#include <ion/gfx/image.h>
#include <ion/gfx/sampler.h>
#include <ion/math/transformutils.h>

#if ENABLE_ION_REMOTE
#include <ion/remote/resourcehandler.h>
#include <ion/remote/settinghandler.h>
#include <ion/remote/shaderhandler.h>
#include <ion/remote/tracinghandler.h>
#endif

#include "Assert.h"
#include "Util/OutputMuter.h"
#include "View.h"

using ion::gfx::FramebufferObject;
using ion::gfx::Image;
using ion::gfx::Sampler;
using ion::gfx::StateTable;
using ion::gfx::Uniform;
using ion::math::Matrix4f;
using ion::math::Point2i;
using ion::math::Point3f;
using ion::math::Range2i;
using ion::math::Vector2i;
using ion::math::Vector3f;
using ion::math::Vector4f;

// ----------------------------------------------------------------------------
// Optional Ion/OpenGL tracing.
// ----------------------------------------------------------------------------

// Turn this on to trace GL calls from Ion.
#define TRACE_GL_ 0

#if TRACE_GL_
#  define TRACE_START_                                                  \
    ion::gfx::TracingStream &s =                                        \
        renderer_->GetGraphicsManager()->GetTracingStream();            \
    s.StartTracing();
#  define TRACE_END_                                                          \
    s.StopTracing();                                                          \
    std::cerr << "GL trace: ========================================\n"       \
              << s.String()                                                   \
              << "==================================================\n";      \
    s.Clear();
#else
#  define TRACE_START_
#  define TRACE_END_
#endif

// ----------------------------------------------------------------------------
// XXXX Move this...
// ----------------------------------------------------------------------------

// Size of depth framebuffer.
static int      kDepthFBSize = 2048;
static Vector3f light_dir(1, -1, -1);

// ----------------------------------------------------------------------------
// Renderer class implementation.
// ----------------------------------------------------------------------------

Renderer::Renderer(const ion::gfxutils::ShaderManagerPtr &shader_manager,
                   const ion::gfx::ShaderProgramPtr &shadow_shader,
                   bool use_ion_remote) :
    shader_manager_(shader_manager),
    is_remote_enabled_(use_ion_remote) {
    display_  = XOpenDisplay(nullptr);
    context_  = glXGetCurrentContext();
    drawable_ = glXGetCurrentDrawable();

    ion::gfx::GraphicsManagerPtr manager(new ion::gfx::GraphicsManager);
    manager->EnableErrorChecking(true);
    renderer_.Reset(new ion::gfx::Renderer(manager));
    frame_.Reset(new ion::gfxutils::Frame);

    // XXXX Set up for shadows.
    ion::gfx::ImagePtr depth_image(new Image);
    depth_image->Set(Image::kRgba8888, kDepthFBSize, kDepthFBSize,
                     ion::base::DataContainerPtr());

    ion::gfx::SamplerPtr sampler(new Sampler);
    sampler->SetMinFilter(Sampler::kLinear);
    sampler->SetMagFilter(Sampler::kLinear);
    sampler->SetWrapS(Sampler::kClampToEdge);
    sampler->SetWrapT(Sampler::kClampToEdge);

    depth_map_texture_.Reset(new ion::gfx::Texture);
    depth_map_texture_->SetSampler(sampler);
    depth_map_texture_->SetLabel("Shadow Depth Map Texture");
    depth_map_texture_->SetImage(0U, depth_image);

    depth_fbo_.Reset(new FramebufferObject(kDepthFBSize, kDepthFBSize));
    depth_fbo_->SetColorAttachment(
        0U, FramebufferObject::Attachment(depth_map_texture_));
    depth_fbo_->SetDepthAttachment(
        FramebufferObject::Attachment(Image::kRenderbufferDepth24));
    depth_fbo_->SetLabel("Shadow Depth FBO");

    depth_map_root_.Reset(new ion::gfx::Node);

    ion::gfx::StateTablePtr state_table(new StateTable(kDepthFBSize,
                                                       kDepthFBSize));
    state_table->SetViewport(
        Range2i::BuildWithSize(Point2i(0, 0),
                               Vector2i(kDepthFBSize, kDepthFBSize)));
    state_table->SetClearColor(Vector4f(1.f, 1.f, 1.f, 1.f));
    state_table->SetClearDepthValue(1.f);
    state_table->Enable(StateTable::kDepthTest, true);
    state_table->Enable(StateTable::kCullFace, true);
    state_table->SetCullFaceMode(StateTable::kCullBack);
    depth_map_root_->SetStateTable(state_table);
    depth_map_root_->SetShaderProgram(shadow_shader);
    auto &reg = shadow_shader->GetRegistry();
    depth_map_root_->AddUniform(
        reg->Create<Uniform>("uLightDir", Vector3f(1, -1, -1)));
    depth_map_root_->AddUniform(
        reg->Create<Uniform>("uBiasMatrix", Matrix4f::Identity()));

#if ENABLE_ION_REMOTE
    SetUpRemoteServer_();
#endif
}

Renderer::~Renderer() {
}

Display *   Renderer::GetDisplay()  const { return display_;  }
GLXContext  Renderer::GetContext()  const { return context_;  }
GLXDrawable Renderer::GetDrawable() const { return drawable_; }

int Renderer::CreateFramebuffer() {
    GLuint fb;
    renderer_->GetGraphicsManager()->GenFramebuffers(1, &fb);
    return fb;
}

void Renderer::RenderView(const View &view, const FBTarget *fb_target) {
    glXMakeCurrent(GetDisplay(), GetDrawable(), GetContext());

    frame_->Begin();
    TRACE_START_;

    // Do shadow passes.

    // Skip over the View's root, which has incorrect matrix stuff.
    depth_map_root_->ClearChildren();
    for (auto &child: view.GetRoot()->GetChildren())
        depth_map_root_->AddChild(child);

    // Update the uBiasMatrix uniform.
    const Matrix4f proj_mat = ion::math::OrthographicMatrixFromFrustum(
        -10.f, 10.f, -10.f, 10.f, -10.f, 20.f);
    const Matrix4f view_mat = ion::math::LookAtMatrixFromDir(
        Point3f(-light_dir), light_dir, Vector3f(0, 1, 0));

    const Matrix4f scale = ion::math::ScaleMatrixH(Vector3f(.5f, .5f, .5f));
    const Matrix4f trans = ion::math::TranslationMatrix(Vector3f(.5f, .5f, .5f));
    const Matrix4f bias_mat = trans * (scale * (proj_mat * view_mat));
    depth_map_root_->SetUniformByName("uBiasMatrix", bias_mat);
    depth_map_root_->SetUniformByName("uLightDir",   light_dir);

    // XXXX
    depth_map_root_->SetUniformByName("uProjectionMatrix", Matrix4f::Identity());
    depth_map_root_->SetUniformByName("uModelviewMatrix", Matrix4f::Identity());

    renderer_->BindFramebuffer(depth_fbo_);
    renderer_->DrawScene(depth_map_root_);

    // ------------------------------------------------------------------

    // Set up the framebuffer(s).
    ion::gfx::GraphicsManager &gm = *renderer_->GetGraphicsManager();
    if (fb_target) {
        ASSERT(fb_target->target_fb >= 0);
        ASSERT(fb_target->color_fb  >  0);
        ASSERT(fb_target->depth_fb  >  0);
        gm.BindFramebuffer(GL_FRAMEBUFFER, fb_target->target_fb);

        gm.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                GL_TEXTURE_2D, fb_target->color_fb, 0);
        gm.FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                GL_TEXTURE_2D, fb_target->depth_fb, 0);
    }
    else {
        renderer_->BindFramebuffer(ion::gfx::FramebufferObjectPtr());
        // XXXX Was: gm.BindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    renderer_->DrawScene(view.GetRoot());

    TRACE_END_;
    frame_->End();

#if ENABLE_ION_REMOTE
    AddNodeTracking(view.GetRoot());
#endif
}

#if ENABLE_ION_REMOTE
void Renderer::SetUpRemoteServer_() {
    if (! is_remote_enabled_)
        return;
    remote_.reset(new ion::remote::RemoteServer(1234));

    ngh_.Reset(new ion::remote::NodeGraphHandler);
    remote_->RegisterHandler(ngh_);
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
}

void Renderer::AddNodeTracking(const ion::gfx::NodePtr &node) {
    ASSERT(node.Get());
    if (is_remote_enabled_ && ! ngh_->IsNodeTracked(node))
        ngh_->AddNode(node);
}
#endif // ENABLE_ION_REMOTE
