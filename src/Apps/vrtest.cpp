#include <iostream>

#include <openvr.h>

#include <ion/gfx/framebufferobject.h>
#include <ion/gfx/image.h>
#include <ion/gfxutils/shadermanager.h>
#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>

#include "App/RegisterTypes.h"
#include "App/Renderer.h"
#include "App/SceneContext.h"
#include "Base/Event.h"
#include "Base/FBTarget.h"
#include "Handlers/MainHandler.h"
#include "Handlers/ViewHandler.h"
#include "IO/Reader.h"
#include "Managers/PrecisionManager.h"
#include "Math/ToString.h"
#include "Math/Types.h"
#include "Panels/Panel.h"

#include "SG/IonContext.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/Tracker.h"
#include "SG/WindowCamera.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/KLog.h"
#include "Viewers/GLFWViewer.h"

// XXXX
static std::string M2S(const Matrix4f &m) { return Math::ToString(m, .001f); }

// XXXX
static void GLMessageCallback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar* message, const void* userParam) {
    fprintf(stderr, "GL: %s type = 0x%x, severity = 0x%x, message = %s\n",
             (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

// ----------------------------------------------------------------------------
// Loader_ class.
// ----------------------------------------------------------------------------

class Loader_ {
  public:
    Loader_();
    SG::ScenePtr LoadScene(const FilePath &path);
    SG::NodePtr  LoadNode(const FilePath &path);

    const ion::gfxutils::ShaderManagerPtr & GetShaderManager() {
        return shader_manager_;
    }

  private:
    SG::TrackerPtr                  tracker_;
    ion::gfxutils::ShaderManagerPtr shader_manager_;
    ion::text::FontManagerPtr       font_manager_;
    SG::IonContextPtr               ion_context_;
};

Loader_::Loader_() : tracker_(new SG::Tracker),
    shader_manager_(new ion::gfxutils::ShaderManager),
    font_manager_(new ion::text::FontManager),
    ion_context_(new SG::IonContext) {

    ion_context_->SetTracker(tracker_);
    ion_context_->SetShaderManager(shader_manager_);
    ion_context_->SetFontManager(font_manager_);
}

SG::ScenePtr Loader_::LoadScene(const FilePath &path) {
    // Wipe out all previous shaders to avoid conflicts.
    shader_manager_.Reset(new ion::gfxutils::ShaderManager);
    ion_context_->Reset();
    ion_context_->SetShaderManager(shader_manager_);

    SG::ScenePtr scene;
    try {
        Reader reader;
        scene = reader.ReadScene(path, *tracker_);
        scene->SetUpIon(ion_context_);
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception loading scene:\n"
                  << ex.what() << "\n";
        scene.reset();
    }
    return scene;
}

// ----------------------------------------------------------------------------
// Application_ class.
// ----------------------------------------------------------------------------

class Application_ {
  public:
    explicit Application_();
    bool InitScene();
    bool InitViewer(const Vector2i &window_size);
    bool InitVR();
    void InitInteraction();
    void MainLoop();

  private:
    /// VR stuff
    struct VREye_ {
        vr::Hmd_Eye   eye;
        FBTarget      fb_target;
        vr::Texture_t tex;

        Rotationf     rotation;     ///< Change in orientation from HMD.
        Vector3f      offset;       ///< Offset in position from HMD.

        Point3f       position;     ///< Absolute position.
        Rotationf     orientation;  ///< Absolute orientation.
    };

    struct VRStuff_ {
        vr::IVRSystem *sys = nullptr;
        uint32         width  = 0;
        uint32         height = 0;
        VREye_         l_eye;
        VREye_         r_eye;
    };

    PrecisionManagerPtr precision_manager_;
    Loader_             loader_;
    SG::ScenePtr        scene_;
    SceneContextPtr     scene_context_;
    SG::WindowCameraPtr camera_;
    GLFWViewerPtr       glfw_viewer_;
    RendererPtr         renderer_;
    MainHandlerPtr      main_handler_;
    ViewHandlerPtr      view_handler_;

    int xxxx_count_ = 0;

    // OpenVR stuff.
    VRStuff_            vr_;

    /// All Handlers, in order.
    std::vector<HandlerPtr> handlers_;

    bool need_render_ = true;
    bool should_quit_ = false;

    void InitVREye_(VREye_ &eye);
    void CreateVRFrameBuffer_(VREye_ &eye);
    void TrackVR_();
    void RenderVREye_(VREye_ &eye);
    bool HandleEvent_(const Event &event);
    void SetUpScene_();
    void UpdateScene_();

    static Matrix4f FromVRMatrix_(const vr::HmdMatrix34_t &m) {
#if 0 // XXXX
        return Matrix4f(m.m[0][0], m.m[1][0], m.m[2][0], 0,
                        m.m[0][1], m.m[1][1], m.m[2][1], 0,
                        m.m[0][2], m.m[1][2], m.m[2][2], 0,
                        m.m[0][3], m.m[1][3], m.m[2][3], 1);
#else // XXXX
        return Matrix4f(m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
                        m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
                        m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
                        0,         0,         0,         1);
#endif // XXX
    }
};

Application_::Application_() {
    RegisterTypes();
}

bool Application_::InitScene() {
    const FilePath scene_path =
        FilePath::GetResourcePath("scenes", "nodeviewer.mvn");
    scene_ = loader_.LoadScene(scene_path);
    if (! scene_)
        return false;
    scene_context_.reset(new SceneContext);

    SetUpScene_();

    return true;
}

bool Application_::InitViewer(const Vector2i &window_size) {
    glfw_viewer_.reset(new GLFWViewer);
    if (! glfw_viewer_->Init(window_size)) {
        glfw_viewer_.reset();
        return false;
    }
    return true;
}

bool Application_::InitVR() {
    // Init VR first without requiring any hardware.
    vr::EVRInitError error = vr::VRInitError_None;
    vr_.sys = vr::VR_Init(&error, vr::VRApplication_Utility);
    if (error != vr::VRInitError_None) {
        std::cerr << "*** Unable to init VR runtime: "
                  << vr::VR_GetVRInitErrorAsEnglishDescription(error) << "\n";
        vr_.sys = nullptr;
        return false;
    }

    // Check for an HMD.
    if (! vr_.sys->IsTrackedDeviceConnected(vr::k_unTrackedDeviceIndex_Hmd)) {
        std::cerr << "*** No HMD connected\n";
        vr::VR_Shutdown();
        vr_.sys = nullptr;
        return false;
    }

    // Shut down and re-init with device.
    vr::VR_Shutdown();
    std::cerr << "XXXX VR Initialized successfully\n";

    // Init VR for real. This has to be done before setting up the Renderer.
    error = vr::VRInitError_None;
    vr_.sys = vr::VR_Init(&error, vr::VRApplication_Scene);
    if (error != vr::VRInitError_None) {
        std::cerr << "*** Unable to init VR runtime: "
                  << vr::VR_GetVRInitErrorAsEnglishDescription(error) << "\n";
        vr_.sys = nullptr;
        return false;
    }
    std::cerr << "XXXX VR Initialized successfully\n";

    if (! vr::VRCompositor()) {
        std::cerr << "*** Compositor initialization failed. "
                  << " See log file for details\n";
        return false;
    }
    std::cerr << "XXXX VRCompositor is ok\n";

    // Set up the eyes.
    vr_.l_eye.eye = vr::Eye_Left;
    vr_.r_eye.eye = vr::Eye_Right;
    InitVREye_(vr_.l_eye);
    InitVREye_(vr_.r_eye);

    return true;
}

void Application_::InitInteraction() {
    precision_manager_.reset(new PrecisionManager);

    // Set up the renderer.
    renderer_.reset(new Renderer(loader_.GetShaderManager(), true));
    renderer_->Reset(*scene_);
    auto &gm = renderer_->GetIonGraphicsManager();
    gm.Enable(GL_DEBUG_OUTPUT);
    gm.DebugMessageCallback(GLMessageCallback, 0);

    main_handler_.reset(new MainHandler);
    main_handler_->SetPrecisionManager(precision_manager_);
    main_handler_->SetSceneContext(scene_context_);

    view_handler_.reset(new ViewHandler);
    view_handler_->SetFixedCameraPosition(true);

    handlers_.push_back(view_handler_);
    handlers_.push_back(main_handler_);

    UpdateScene_();

    // Set up VR for rendering.
    vr_.sys->GetRecommendedRenderTargetSize(&vr_.width, &vr_.height);
    CreateVRFrameBuffer_(vr_.l_eye);
    CreateVRFrameBuffer_(vr_.r_eye);
}

void Application_::MainLoop() {
    std::vector<Event> events;
    while (! should_quit_) {
        renderer_->BeginFrame();
        if (xxxx_count_ < 20)
            std::cerr << "XXXX ------------------ FRAME " << xxxx_count_ << "\n";
        // else break;  // XXXX

        const bool is_alternate_mode = glfw_viewer_->IsShiftKeyPressed();

        // Update the frustum used for intersection testing.
        scene_context_->frustum = glfw_viewer_->GetFrustum();
        if (xxxx_count_ < 2) {
            std::cerr << "XXXX GL frust = "
                      << scene_context_->frustum.ToString() << "\n";
        }

        main_handler_->ProcessUpdate(is_alternate_mode);

        events.clear();
        glfw_viewer_->SetPollEventsFlag(true);  // Always poll so VR is updated.
        glfw_viewer_->EmitEvents(events);
        for (auto &event: events) {
            if (event.flags.Has(Event::Flag::kExit)) {
                should_quit_ = true;
                break;
            }

            if (! HandleEvent_(event)) {
                for (auto &handler: handlers_)
                    if (handler->HandleEvent(event))
                        break;
            }
        }

        // Render to all viewers.
        need_render_ = false;
        glfw_viewer_->Render(*scene_, *renderer_);

        // Update VR tracking.
        TrackVR_();

        RenderVREye_(vr_.l_eye);
        RenderVREye_(vr_.r_eye);

        // XXXX Move into RenderVREye_() if possible.
        vr::VRCompositor()->Submit(vr_.l_eye.eye, &vr_.l_eye.tex);
        vr::VRCompositor()->Submit(vr_.r_eye.eye, &vr_.r_eye.tex);

        auto &gm = renderer_->GetIonGraphicsManager();
        if (false) { // XXXX
            gm.ClearColor(0, 0, 0, 1);
            gm.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        gm.Flush();
        gm.Finish();  // XXXX Needed to avoid hang?

        ++xxxx_count_;
        renderer_->EndFrame();
    }

    vr::VR_Shutdown();
}

void Application_::InitVREye_(VREye_ &eye) {
    eye.tex.eType       = vr::TextureType_OpenGL;
    eye.tex.eColorSpace = vr::ColorSpace_Gamma;

    const Matrix4f eye_to_head =
        FromVRMatrix_(vr_.sys->GetEyeToHeadTransform(eye.eye));

    const Matrix4f head_to_eye = ion::math::Inverse(
        FromVRMatrix_(vr_.sys->GetEyeToHeadTransform(eye.eye)));

    eye.offset   = Vector3f(head_to_eye * Point3f::Zero());
    eye.rotation = Rotationf::FromRotationMatrix(
        ion::math::GetRotationMatrix(head_to_eye));

    std::cerr << "XXXX " << Util::EnumName(eye.eye)
              << " E2H=\n" << M2S(eye_to_head) << "\n";

    std::cerr << "XXXX " << Util::EnumName(eye.eye)
              << " H2E=\n" << M2S(head_to_eye) << "\n";

    std::cerr << "XXXX " << Util::EnumName(eye.eye)
              << " off=" << eye.offset
              << " rot=" << eye.rotation << "\n";
}

void Application_::CreateVRFrameBuffer_(VREye_ &eye) {
    using ion::gfx::FramebufferObjectPtr;
    using ion::gfx::FramebufferObject;
    using ion::gfx::Image;

    const std::string eye_str = std::string("VR ") +
        (eye.eye == vr::Eye_Left ? "L" : "R") + " Eye ";

    ASSERT(renderer_);

    const auto w = vr_.width;
    const auto h = vr_.height;
    const int kSampleCount = 4;

    // Create Images in which to store color and depth values.
    ion::gfx::ImagePtr color_image(new ion::gfx::Image);
    ion::gfx::ImagePtr depth_image(new ion::gfx::Image);
    color_image->Set(ion::gfx::Image::kRgba8888, w, h,
                     ion::base::DataContainerPtr());
    depth_image->Set(ion::gfx::Image::kRenderbufferDepth24, w, h,
                     ion::base::DataContainerPtr());

    // Create a Sampler for the textures.
    ion::gfx::SamplerPtr sampler(new ion::gfx::Sampler);
    sampler->SetMinFilter(ion::gfx::Sampler::kLinear);
    sampler->SetMagFilter(ion::gfx::Sampler::kLinear);
    sampler->SetWrapS(ion::gfx::Sampler::kClampToEdge);
    sampler->SetWrapT(ion::gfx::Sampler::kClampToEdge);

    // Create the color and depth textures.
    ion::gfx::TexturePtr color_tex(new ion::gfx::Texture);
    ion::gfx::TexturePtr depth_tex(new ion::gfx::Texture);
    color_tex->SetLabel(eye_str + "Color Texture");
    depth_tex->SetLabel(eye_str + "Depth Texture");
    color_tex->SetSampler(sampler);
    depth_tex->SetSampler(sampler);
    color_tex->SetImage(0U, color_image);
    depth_tex->SetImage(0U, depth_image);

    // Render FBO with multisampled color and depth attachments.
    auto &rend_fbo = eye.fb_target.rend_fbo;
    rend_fbo.Reset(new FramebufferObject(w, h));
    rend_fbo->SetLabel(eye_str + "Render FBO");
    rend_fbo->SetColorAttachment(
        0U, FramebufferObject::Attachment::CreateImplicitlyMultisampled(
            color_tex, kSampleCount));
    rend_fbo->SetDepthAttachment(
        FramebufferObject::Attachment::CreateImplicitlyMultisampled(
            depth_tex, kSampleCount));

    // Destination image and texture.
    ion::gfx::ImagePtr dest_image(new ion::gfx::Image);
    dest_image->Set(ion::gfx::Image::kRgba8888, w, h,
                    ion::base::DataContainerPtr());
    ion::gfx::TexturePtr dest_tex(new ion::gfx::Texture);
    dest_tex->SetLabel(eye_str + "Dest Texture");
    dest_tex->SetSampler(sampler);
    dest_tex->SetImage(0U, dest_image);

    // Destination FBO.
    auto &dest_fbo = eye.fb_target.dest_fbo;
    dest_fbo.Reset(new FramebufferObject(w, h));
    dest_fbo->SetLabel(eye_str + "Dest FBO");
    dest_fbo->SetColorAttachment(0U, FramebufferObject::Attachment(dest_tex));

#if XXXX
    auto &gm = renderer_->GetIonGraphicsManager();

    std::cerr << "XXXX CreateVRFrameBuffer_ for " << Util::EnumName(eye.eye)
              << " W=" << vr_.width << " H=" << vr_.height << "\n";

    auto &fbt = eye.fb_target;

    auto gen_fb = [&]{
        GLuint id;
        gm.GenFramebuffers(1, &id);
        return id;
    };
    auto gen_tex = [&]{
        GLuint id;
        gm.GenTextures(1, &id);
        return id;
    };

    fbt.render_framebuffer_id = gen_fb();
    gm.BindFramebuffer(GL_FRAMEBUFFER, fbt.render_framebuffer_id);

    fbt.depth_buffer_id = gen_fb();
    gm.BindRenderbuffer(GL_RENDERBUFFER, fbt.depth_buffer_id);
    gm.RenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT,
                                      vr_.width, vr_.height);
    gm.FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_RENDERBUFFER,	fbt.depth_buffer_id);

    fbt.render_texture_id = gen_tex();
    gm.BindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbt.render_texture_id);
    gm.TexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8,
                             vr_.width, vr_.height, true);
    gm.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_2D_MULTISAMPLE,
                            fbt.render_texture_id, 0);

    fbt.resolve_framebuffer_id = gen_fb();
    gm.BindFramebuffer(GL_FRAMEBUFFER, fbt.resolve_framebuffer_id);

    fbt.resolve_texture_id = gen_tex();
    gm.BindTexture(GL_TEXTURE_2D, fbt.resolve_texture_id);
    gm.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gm.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    gm.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, vr_.width, vr_.height,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    gm.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_2D, fbt.resolve_texture_id, 0);

    if (gm.CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "*** Error creating VR framebuffer\n";

    gm.BindFramebuffer(GL_FRAMEBUFFER, 0);

    eye.tex.handle = reinterpret_cast<void *>(fbt.resolve_texture_id);
#endif
}

void Application_::TrackVR_() {
    const uint32 kCount = vr::k_unMaxTrackedDeviceCount;
    vr::TrackedDevicePose_t poses[kCount];
    if (vr::VRCompositor()->WaitGetPoses(poses, kCount, nullptr, 0) !=
        vr::VRCompositorError_None) {
        std::cerr << "*** Unable to get poses\n";
    }

    // Update the position and orientation for each eye from the HMD pose data.
    const auto &hmd_pose = poses[vr::k_unTrackedDeviceIndex_Hmd];
    if (hmd_pose.bPoseIsValid) {
        const Matrix4f m = ion::math::Inverse(
            FromVRMatrix_(hmd_pose.mDeviceToAbsoluteTracking));

        const Point3f camera_position(0, 10.55f, -60); // XXXX Get from scene.

        vr_.l_eye.position =
            camera_position + m * Point3f::Zero() + vr_.l_eye.offset;
        vr_.l_eye.orientation =
            Rotationf::FromRotationMatrix(ion::math::GetRotationMatrix(m)) *
            vr_.l_eye.rotation;

        vr_.r_eye.position =
            camera_position + m * Point3f::Zero() + vr_.r_eye.offset;
        vr_.r_eye.orientation =
            Rotationf::FromRotationMatrix(ion::math::GetRotationMatrix(m)) *
            vr_.r_eye.rotation;

        if (xxxx_count_ < 2) {
            std::cerr << "XXXX M=\n" << M2S(m) << "\n";
            std::cerr << "XXXX L eye pos=" << vr_.l_eye.position
                      << " or=" << vr_.l_eye.orientation << "\n";
        }
    }

    // XXXX Get controller positions and orientations.
}

void Application_::RenderVREye_(VREye_ &eye) {
    // Set up the viewing frustum for the eye.
    const float kNear = 0.1f;
    const float kFar  = 300.0f;
    Frustum frustum;
    float left, right, down, up;
    vr_.sys->GetProjectionRaw(eye.eye, &left, &right, &down, &up);
    frustum.SetFromTangents(left, right, down, up);
    frustum.pnear = kNear;
    frustum.pfar  = kFar;
    frustum.viewport = Viewport::BuildWithSize(Point2i(0, 0),
                                               Vector2i(vr_.width, vr_.height));
    frustum.position    = eye.position;
    frustum.orientation = eye.orientation;

    // XXXX Copy GLFWViewer frustum (except viewport) for debugging...
    if (false) { // XXXX
        const auto vp = frustum.viewport;
        frustum = glfw_viewer_->GetFrustum();
        frustum.viewport = vp;
    }

    renderer_->RenderScene(*scene_, frustum, &eye.fb_target);

    auto &ion_renderer = renderer_->GetIonRenderer(); // XXXX
    auto dest_id = ion_renderer.GetResourceGlId(eye.fb_target.dest_fbo.Get());
    eye.tex.handle = reinterpret_cast<void *>(dest_id);
}

bool Application_::HandleEvent_(const Event &event) {
    // Scroll wheel zooms in and out.
    if (event.flags.Has(Event::Flag::kPosition1D) &&
        event.device == Event::Device::kMouse) {
        const float kDeltaFOV = 2;
        const float kMinFOV   = 40;
        const float kMaxFOV   = 120;
        float fov = camera_->GetFOV().Degrees();
        fov -= event.position1D * kDeltaFOV;
        fov = Clamp(fov, kMinFOV, kMaxFOV);
        camera_->SetFOV(Anglef::FromDegrees(fov));
        return true;
    }

    // Handle key presses.
    if (event.flags.Has(Event::Flag::kKeyPress)) {
        const std::string key_string = event.GetKeyString();
        if (key_string == "<Ctrl>q") {
            should_quit_ = true;
            return true;
        }
    }

    return false;
}

void Application_::SetUpScene_() {
    ASSERT(scene_);
    ASSERT(scene_context_);
    scene_context_->FillFromScene(scene_, false);
    scene_context_->path_to_stage = SG::NodePath(scene_->GetRootNode());

    // Now that everything has been found, disable searching through the
    // "Definitions" Node.
    SG::FindNodeInScene(*scene_, "Definitions")->SetFlagEnabled(
        SG::Node::Flag::kSearch, false);

    // Check for changes to the root node to trigger rendering.
    scene_->GetRootNode()->GetChanged().AddObserver(
        this, [&](SG::Change, const SG::Object &){ need_render_ = true; });
}

void Application_::UpdateScene_() {
    ASSERT(scene_);

    // Install the window camera in the viewer.
    auto gantry = scene_->GetGantry();
    ASSERT(gantry);
    for (auto &cam: gantry->GetCameras()) {
        if (cam->GetTypeName() == "WindowCamera") {
            camera_ = Util::CastToDerived<SG::WindowCamera>(cam);
            break;
        }
    }
    ASSERT(camera_);
    view_handler_->SetCamera(camera_);
    glfw_viewer_->SetCamera(camera_);
}

// ----------------------------------------------------------------------------
// Mainline.
// ----------------------------------------------------------------------------

int main(int argc, const char** argv)
{
    KLogger::SetKeyString("");

    Application_ app;
    try {
        if (! app.InitScene() ||
            ! app.InitViewer(Vector2i(800, 600)) ||
            ! app.InitVR())
            return 1;
        app.InitInteraction();
        app.MainLoop();
    }
    catch (AssertException &ex) {
        std::cerr << "*** Caught assertion exception:\n" << ex.what() << "\n";
    }
    return 0;
}
