#include <iostream>

#include <openvr.h>

#include <ion/gfx/framebufferobject.h>
#include <ion/gfx/image.h>
#include <ion/gfxutils/shadermanager.h>
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

    struct VRHandActions_ {
        vr::VRActionHandle_t pinch_action = vr::k_ulInvalidActionHandle;
        vr::VRActionHandle_t grip_action  = vr::k_ulInvalidActionHandle;
        vr::VRActionHandle_t menu_action  = vr::k_ulInvalidActionHandle;
        vr::VRActionHandle_t thumb_action = vr::k_ulInvalidActionHandle;
        vr::VRActionHandle_t pose_action  = vr::k_ulInvalidActionHandle;
    };

    struct VRStuff_ {
        vr::IVRSystem *sys = nullptr;
        uint32         width  = 0;
        uint32         height = 0;

        // Rendering.
        VREye_         l_eye;
        VREye_         r_eye;

        // Input.
        vr::VRActiveActionSet_t action_set;
        VRHandActions_          l_actions;
        VRHandActions_          r_actions;
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

    void InitVREye_(VREye_ &eye);
    void CreateVRFrameBuffer_(VREye_ &eye);
    void InitVRInput_();
    void InitVRHandActions_(const std::string &hand, VRHandActions_ &actions);
    void TrackVR_();
    void RenderVREye_(VREye_ &eye);
    bool HandleEvent_(const Event &event);
    bool ActionChanged_(vr::VRActionHandle_t action);
    void SetUpScene_();
    void UpdateScene_();

    static Matrix4f FromVRMatrix_(const vr::HmdMatrix34_t &m) {
        return Matrix4f(m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
                        m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
                        m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
                        0,         0,         0,         1);
    }

    static Matrix4f FromVRMatrix_(const vr::HmdMatrix44_t &m) {
        return Matrix4f(m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
                        m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
                        m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
                        m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3]);
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

    // Init VR for real. This has to be done before setting up the Renderer.
    error = vr::VRInitError_None;
    vr_.sys = vr::VR_Init(&error, vr::VRApplication_Scene);
    if (error != vr::VRInitError_None) {
        std::cerr << "*** Unable to init VR runtime: "
                  << vr::VR_GetVRInitErrorAsEnglishDescription(error) << "\n";
        vr_.sys = nullptr;
        return false;
    }

    if (! vr::VRCompositor()) {
        std::cerr << "*** Compositor initialization failed. "
                  << " See log file for details\n";
        return false;
    }

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
    const bool do_remote = false;
    renderer_.reset(new Renderer(loader_.GetShaderManager(), do_remote));
    renderer_->Reset(*scene_);

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

    // And for input.
    InitVRInput_();
}

void Application_::MainLoop() {
    std::vector<Event> events;
    while (! should_quit_) {
        renderer_->BeginFrame();

        const bool is_alternate_mode = glfw_viewer_->IsShiftKeyPressed();

        // Update the frustum used for intersection testing.
        scene_context_->frustum = glfw_viewer_->GetFrustum();

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
        if (need_render_) {
            glfw_viewer_->Render(*scene_, *renderer_);
            need_render_ = false;
        }

        RenderVREye_(vr_.l_eye);
        RenderVREye_(vr_.r_eye);

        renderer_->EndFrame();

        // Update VR tracking and input.
        TrackVR_();
    }

    vr::VR_Shutdown();
}

void Application_::InitVREye_(VREye_ &eye) {
    eye.tex.eType       = vr::TextureType_OpenGL;
    eye.tex.eColorSpace = vr::ColorSpace_Gamma;

    // Note that GetEyeToHeadTransform() actually returns a matrix that
    // converts from head coordinates to eye coordinates. Hmmmm.

    const Matrix4f head_to_eye =
        FromVRMatrix_(vr_.sys->GetEyeToHeadTransform(eye.eye));

    eye.offset   = Vector3f(head_to_eye * Point3f::Zero());
    eye.rotation = Rotationf::FromRotationMatrix(
        ion::math::GetRotationMatrix(head_to_eye));
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

    // Render FBO with multisampled color and depth attachments.
    auto &rend_fbo = eye.fb_target.rend_fbo;
    rend_fbo.Reset(new FramebufferObject(w, h));
    rend_fbo->SetLabel(eye_str + "Render FBO");
    rend_fbo->SetColorAttachment(
        0U, FramebufferObject::Attachment::CreateMultisampled(
            ion::gfx::Image::kRgba8888, kSampleCount));
    rend_fbo->SetDepthAttachment(
        FramebufferObject::Attachment::CreateMultisampled(
            ion::gfx::Image::kRenderbufferDepth16, kSampleCount));

    // Destination sampler, image, and texture.
    ion::gfx::SamplerPtr sampler(new ion::gfx::Sampler);
    sampler->SetMinFilter(ion::gfx::Sampler::kLinear);
    sampler->SetMagFilter(ion::gfx::Sampler::kLinear);
    sampler->SetWrapS(ion::gfx::Sampler::kClampToEdge);
    sampler->SetWrapT(ion::gfx::Sampler::kClampToEdge);
    ion::gfx::ImagePtr dest_image(new ion::gfx::Image);
    dest_image->Set(ion::gfx::Image::kRgba8888, w, h,
                    ion::base::DataContainerPtr());
    ion::gfx::TexturePtr dest_tex(new ion::gfx::Texture);
    dest_tex->SetLabel(eye_str + "Dest Texture");
    dest_tex->SetSampler(sampler);
    dest_tex->SetMaxLevel(0);
    dest_tex->SetImage(0U, dest_image);

    // Destination FBO.
    auto &dest_fbo = eye.fb_target.dest_fbo;
    dest_fbo.Reset(new FramebufferObject(w, h));
    dest_fbo->SetLabel(eye_str + "Dest FBO");
    dest_fbo->SetColorAttachment(0U, FramebufferObject::Attachment(dest_tex));
}

void Application_::InitVRInput_() {
    auto &vin = *vr::VRInput();

    const auto manifest_path =
        FilePath::GetResourcePath("json", "imakervr_actions.json");
    const auto merr =
        vin.SetActionManifestPath(manifest_path.ToString().c_str());
    if (merr != vr::VRInputError_None) {
        std::cerr << "*** Error setting manifest path: " << Util::EnumName(merr);
    }

    auto &action_set = vr_.action_set;
    const auto err = vin.GetActionSetHandle("/actions/default",
                                            &action_set.ulActionSet);
    if (err != vr::VRInputError_None) {
        std::cerr << "*** Error getting action set: " << Util::EnumName(err);
    }
    action_set.ulRestrictedToDevice = vr::k_ulInvalidInputValueHandle;
    action_set.nPriority = 0;

    InitVRHandActions_("Left",  vr_.l_actions);
    InitVRHandActions_("Right", vr_.r_actions);
}

void Application_::InitVRHandActions_(const std::string &hand,
                                      VRHandActions_ &actions) {
    auto &vin = *vr::VRInput();

    auto get_action = [&](const std::string &name,
                          vr::VRActionHandle_t &action) {
        const std::string &path = "/actions/default/in/" + hand + name;
        const auto err = vin.GetActionHandle(path.c_str(), &action);
        if (err != vr::VRInputError_None) {
            std::cerr << "*** Error getting action for path '" << path
                      << "': " << Util::EnumName(err);
        }
    };

    get_action("Pinch", actions.pinch_action);
    get_action("Grip",  actions.grip_action);
    get_action("Menu",  actions.menu_action);
    get_action("Thumb", actions.thumb_action);
    get_action("Pose",  actions.pose_action);
}

void Application_::TrackVR_() {
    // Process SteamVR events
    vr::VREvent_t event;
    while (vr_.sys->PollNextEvent(&event, sizeof(event))) {
        const vr::EVREventType type =
            static_cast<vr::EVREventType>(event.eventType);
        if (type == vr::VREvent_Input_BindingLoadFailed ||
            type == vr::VREvent_Input_BindingLoadSuccessful) {
            std::cerr << "XXXX Got VR event of type "
                      << vr_.sys->GetEventTypeNameFromEnum(type)
                      << " (" << event.eventType << ")\n";
        }
    }

    const uint32 kCount = vr::k_unMaxTrackedDeviceCount;
    vr::TrackedDevicePose_t poses[kCount];
    if (vr::VRCompositor()->WaitGetPoses(poses, kCount, nullptr, 0) !=
        vr::VRCompositorError_None) {
        std::cerr << "*** Unable to get poses\n";
    }

    // Update the position and orientation for each eye from the HMD pose data.
    const auto &hmd_pose = poses[vr::k_unTrackedDeviceIndex_Hmd];
    if (hmd_pose.bPoseIsValid) {
        const Point3f camera_position(0, 10.55f, 60); // XXXX Get from scene.

        const Matrix4f pose = FromVRMatrix_(hmd_pose.mDeviceToAbsoluteTracking);
        const Rotationf rot = Rotationf::FromRotationMatrix(
            ion::math::GetRotationMatrix(pose));

        const Point3f head_pos = camera_position + pose * Point3f::Zero();

        vr_.l_eye.position    = head_pos + pose * vr_.l_eye.offset;
        vr_.r_eye.position    = head_pos + pose * vr_.r_eye.offset;
        vr_.l_eye.orientation = rot * vr_.l_eye.rotation;
        vr_.r_eye.orientation = rot * vr_.r_eye.rotation;
    }

    auto &vin = *vr::VRInput();
    vin.GetActionSetHandle("/actions/default", &vr_.action_set.ulActionSet);
    const auto err =
        vin.UpdateActionState(&vr_.action_set, sizeof(vr_.action_set), 1);
    if (err != vr::VRInputError_None) {
        std::cerr << "*** Error updating action state: " << Util::EnumName(err);
    }

    // XXXX Get controller positions and orientations.

    // Check for input button changes.
    if (ActionChanged_(vr_.l_actions.pinch_action))
        std::cerr << "XXXX Left pinch!\n";
    if (ActionChanged_(vr_.r_actions.pinch_action))
        std::cerr << "XXXX Right pinch!\n";
    if (ActionChanged_(vr_.l_actions.grip_action))
        std::cerr << "XXXX Left grip!\n";
    if (ActionChanged_(vr_.r_actions.grip_action))
        std::cerr << "XXXX Right grip!\n";
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

    renderer_->RenderScene(*scene_, frustum, &eye.fb_target);

    auto &ion_renderer = renderer_->GetIonRenderer(); // XXXX
    auto &ca = eye.fb_target.dest_fbo->GetColorAttachment(0);
    ASSERT(ca.GetTexture().Get());
    auto dest_tex_id = ion_renderer.GetResourceGlId(ca.GetTexture().Get());
    eye.tex.handle = reinterpret_cast<void *>(dest_tex_id);

    vr::VRCompositor()->Submit(eye.eye, &eye.tex);
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

bool Application_::ActionChanged_(vr::VRActionHandle_t action) {
    ASSERT(action != vr::k_ulInvalidActionHandle);

    auto &vin = *vr::VRInput();

    vr::InputDigitalActionData_t data;
    auto err = vin.GetDigitalActionData(action, &data, sizeof(data),
                                        vr::k_ulInvalidInputValueHandle);
    if (err != vr::VRInputError_None) {
        std::cerr << "*** Error getting data for action: "
                  << Util::EnumName(err) << "\n";
    }
    return data.bActive && data.bChanged && data.bState;
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
