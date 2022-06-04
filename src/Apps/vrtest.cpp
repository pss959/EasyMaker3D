#include <iostream>

#include <openvr.h>

#include <ion/gfxutils/shadermanager.h>

#include "App/RegisterTypes.h"
#include "App/Renderer.h"
#include "App/SceneContext.h"
#include "Base/Event.h"
#include "Handlers/MainHandler.h"
#include "Handlers/ViewHandler.h"
#include "IO/Reader.h"
#include "Managers/PrecisionManager.h"
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
    // VR stuff
    struct VREye_ {
        GLuint depth_buffer_id        = 0;
        GLuint render_texture_id      = 0;
        GLuint render_framebuffer_id  = 0;
        GLuint resolve_texture_id     = 0;
        GLuint resolve_framebuffer_id = 0;
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

    // OpenVR stuff.
    VRStuff_            vr_;

    /// All Handlers, in order.
    std::vector<HandlerPtr> handlers_;

    bool need_render_ = true;
    bool should_quit_ = false;

    void CreateVRFrameBuffer_(VREye_ &eye);
    void RenderVR_();
    void RenderVREye_(vr::Hmd_Eye eye);

    bool HandleEvent_(const Event &event);
    void SetUpScene_();
    void UpdateScene_();
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
    // Init VR. This has to be done before setting up the Renderer.
    vr::EVRInitError error = vr::VRInitError_None;
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

    return true;
}

void Application_::InitInteraction() {
    precision_manager_.reset(new PrecisionManager);

    // Set up the renderer.
    renderer_.reset(new Renderer(loader_.GetShaderManager(), true));
    renderer_->Reset(*scene_);

    main_handler_.reset(new MainHandler);
    main_handler_->SetPrecisionManager(precision_manager_);
    main_handler_->SetSceneContext(scene_context_);

    view_handler_.reset(new ViewHandler);
    view_handler_->SetFixedCameraPosition(false);

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
        const bool is_alternate_mode = glfw_viewer_->IsShiftKeyPressed();

        // Update the frustum used for intersection testing.
        scene_context_->frustum = glfw_viewer_->GetFrustum();

        main_handler_->ProcessUpdate(is_alternate_mode);

        events.clear();
        glfw_viewer_->SetPollEventsFlag(need_render_ ||
                                        ! main_handler_->IsWaiting());
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

        RenderVR_();
    }
}

void Application_::CreateVRFrameBuffer_(VREye_ &eye) {
    ASSERT(renderer_);

    ion::gfx::GraphicsManager &gm = renderer_->GetIonGraphicsManager();

    gm.GenFramebuffers(1, &eye.render_framebuffer_id);
    gm.BindFramebuffer(GL_FRAMEBUFFER, eye.render_framebuffer_id);

    gm.GenRenderbuffers(1, &eye.depth_buffer_id);
    gm.BindRenderbuffer(GL_RENDERBUFFER, eye.depth_buffer_id);
    gm.RenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT,
                                      vr_.width, vr_.height);
    gm.FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_RENDERBUFFER,	eye.depth_buffer_id);

    gm.GenTextures(1, &eye.render_texture_id);
    gm.BindTexture(GL_TEXTURE_2D_MULTISAMPLE, eye.render_texture_id);
    gm.TexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8,
                             vr_.width, vr_.height, true);
    gm.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_2D_MULTISAMPLE,
                            eye.render_texture_id, 0);

    gm.GenFramebuffers(1, &eye.resolve_framebuffer_id);
    gm.BindFramebuffer(GL_FRAMEBUFFER, eye.resolve_framebuffer_id);

    gm.GenTextures(1, &eye.resolve_texture_id);
    gm.BindTexture(GL_TEXTURE_2D, eye.resolve_texture_id);
    gm.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gm.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    gm.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, vr_.width, vr_.height,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    gm.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_2D, eye.resolve_texture_id, 0);

    if (gm.CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "*** Error creating VR framebuffer\n";

    gm.BindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application_::RenderVR_() {
#if XXXX
XXXX   CMainApplication::RenderFrame
XXXX     CMainApplication::RenderStereoTargets
XXXX       CMainApplication::RenderScene
XXXX         CMainApplication::GetCurrentViewProjectionMatrix
XXXX         CMainApplication::GetCurrentViewProjectionMatrix
XXXX       CMainApplication::RenderScene
XXXX         CMainApplication::GetCurrentViewProjectionMatrix
XXXX         CMainApplication::GetCurrentViewProjectionMatrix
#endif

    RenderVREye_(vr::Eye_Left);
    RenderVREye_(vr::Eye_Right);
}

void Application_::RenderVREye_(vr::Hmd_Eye eye) {
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
