#include "VR/VRContext.h"

#include <openvr.h>

#include <ion/gfx/framebufferobject.h>
#include <ion/gfx/image.h>
#include <ion/math/transformutils.h>

#include "App/Renderer.h"
#include "Base/Event.h"
#include "Base/FBTarget.h"
#include "Enums/Hand.h"
#include "Math/Linear.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/FilePath.h"
#include "Util/KLog.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

/// Converts a 3x4 OpenVR matrix to a Matrix4f.
static Matrix4f ConvertMatrix_(const vr::HmdMatrix34_t &m) {
    return Matrix4f(m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
                    m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
                    m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
                    0,         0,         0,         1);
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// VRContext::Impl_ class.
// ----------------------------------------------------------------------------

/// This class does most of the work for the VRContext.
class VRContext::Impl_ {
  public:
    bool InitSystem();
    void InitRendering(Renderer &renderer);
    void Render(const SG::Scene &scene, Renderer &renderer,
                const Point3f &base_position);
    void EmitEvents(std::vector<Event> &events, const Point3f &base_position);
    void Shutdown();

  private:
    /// Tracked VR headset and controller buttons.
    enum class Button_ {
        kHeadsetOnHead,
        kPinch,
        kGrip,
        kMenu,
        kCenter,
        kLeft,
        kRight,
        kUp,
        kDown,
    };

    /// Information required to render to each eye.
    struct Eye_ {
        vr::Hmd_Eye   eye;          ///< OpenVR eye enum.
        vr::Texture_t texture;      ///< OpenVR resolved texture descriptor.
        Rotationf     rotation;     ///< Difference in orientation from HMD.
        Vector3f      offset;       ///< Difference in position from HMD.
        Point3f       position;     ///< Absolute position.
        Rotationf     orientation;  ///< Absolute orientation.
        FBTarget      fb_target;    ///< For passing framebuffers to Renderer.
    };

    /// OpenVR action handles.
    struct Actions_ {
        vr::VRActionHandle_t buttons[Util::EnumCount<Button_>()];
        vr::VRActionHandle_t thumb_pos;
        vr::VRActionHandle_t hand_poses[2];  // Indexed by Hand enum.
    };

    /// Information about each controller.
    struct Controller_ {
        Hand          hand;    ///< Corresponding Hand.
        Event::Device device;  ///< Corresponding device.

        vr::VRInputValueHandle_t handle = vr::k_ulInvalidInputValueHandle;

        /// This saves the position of the controller at the previous frame so
        /// that the motion can be computed.
        Point3f prev_position{0, 0, 0};
    };

    // Rendering.
    Vector2ui window_size_;
    Eye_      l_eye_;
    Eye_      r_eye_;

    // HMD.
    Point3f   head_pos_;               ///< Current HMD position.
    bool      is_headset_on_ = false;  ///< True if HMD is being worn.

    // Devices.
    vr::VRInputValueHandle_t headset_handle_ = vr::k_ulInvalidInputValueHandle;
    Controller_              controllers_[2];  // Indexed by Hand enum.

    // Input.
    vr::VRActiveActionSet_t action_set_;
    Actions_                actions_;

    bool CheckForHMD_();
    bool InitVR_();
    void InitEye_(vr::Hmd_Eye which_eye, Eye_ &eye);
    void InitInput_();
    void LoadActions_();
    void InitActionSet_();
    void InitActions_();
    void InitEyeRendering_(Renderer &renderer, Eye_ &eye);
    void UpdateEyes_(const Point3f &base_position);
    void RenderEye_(Eye_ &eye, const SG::Scene &scene, Renderer &renderer);
    void AddButtonEvents_(Button_ but, std::vector<Event> &events);
    void AddHandPoseEvent_(Hand hand, std::vector<Event> &events,
                           const Point3f &base_position);
    bool GetButtonState_(Button_ but);
    static Event::Button GetEventButton_(Button_ but);
};

// ----------------------------------------------------------------------------
// VRContext::Impl_ functions.
// ----------------------------------------------------------------------------

bool VRContext::Impl_::InitSystem() {
    KLOG('v', "Initializing VR system");

    // Init VR without requiring hardware to see if there is an HMD present. If
    // that passes, init for real to make sure everything works as expected.
    if (! CheckForHMD_() || ! InitVR_())
        return false;

    // Set up the eyes.
    InitEye_(vr::Eye_Left,  l_eye_);
    InitEye_(vr::Eye_Right, r_eye_);

    // Set up input.
    InitInput_();

    return true;
}

void VRContext::Impl_::InitRendering(Renderer &renderer) {
    KLOG('v', "Initializing VR rendering");

    // Set up VR for rendering.
    auto &sys = *vr::VRSystem();

    sys.GetRecommendedRenderTargetSize(&window_size_[0], &window_size_[1]);
    KLOG('v', "VR window size is "
         << window_size_[0] << " x " << window_size_[1]);

    InitEyeRendering_(renderer, l_eye_);
    InitEyeRendering_(renderer, r_eye_);
}

void VRContext::Impl_::Render(const SG::Scene &scene, Renderer &renderer,
                              const Point3f &base_position) {
    // Make sure the eyes are positioned and rotated correctly.
    UpdateEyes_(base_position);

    // Render each eye into textures.
    RenderEye_(l_eye_, scene, renderer);
    RenderEye_(r_eye_, scene, renderer);
}

void VRContext::Impl_::EmitEvents(std::vector<Event> &events,
                                  const Point3f &base_position) {

    auto &vin = *vr::VRInput();

    if (vin.UpdateActionState(&action_set_, sizeof(action_set_), 1) !=
        vr::VRInputError_None) {
        std::cerr << "*** Error updating action state\n";
    }

    // Determine if the headset is currently on. This needs to be set before
    // the hand poses can be processed properly.
    is_headset_on_ = GetButtonState_(Button_::kHeadsetOnHead);

    // Check for input button changes.
    for (auto but: Util::EnumValues<Button_>())
        AddButtonEvents_(but, events);

    // Always update controller positions.
    for (auto hand: Util::EnumValues<Hand>())
        AddHandPoseEvent_(hand, events, base_position);
}

void VRContext::Impl_::Shutdown() {
    KLOG('v', "Shutting down VR");
    vr::VR_Shutdown();
}

bool VRContext::Impl_::CheckForHMD_() {
    // Init without requiring any hardware.
    /// \todo Figure out if there is a way to stifle messages from this.
    auto error = vr::VRInitError_None;
    auto sys = vr::VR_Init(&error, vr::VRApplication_Utility);
    if (error != vr::VRInitError_None) {
        ASSERT(! sys);
        KLOG('v', "***Unable to init VR runtime: "
             << vr::VR_GetVRInitErrorAsEnglishDescription(error));
        return false;
    }

    // See if there is an HMD.
    ASSERT(sys);
    if (! sys->IsTrackedDeviceConnected(vr::k_unTrackedDeviceIndex_Hmd)) {
        KLOG('v', "***No HMD connected");
        vr::VR_Shutdown();
        return false;
    }

    // Shut down so the real initialization will work.
    vr::VR_Shutdown();
    return true;
}

bool VRContext::Impl_::InitVR_() {
    auto error = vr::VRInitError_None;
    vr::VR_Init(&error, vr::VRApplication_Scene);
    if (error != vr::VRInitError_None) {
        KLOG('v', "***Unable to init VR runtime: "
             << vr::VR_GetVRInitErrorAsEnglishDescription(error));
        vr::VR_Shutdown();
        return false;
    }

    if (! vr::VRCompositor()) {
        KLOG('v', "***Compositor initialization failed. "
             " See log file for details");
        vr::VR_Shutdown();
        return false;
    }

    return true;
}

void VRContext::Impl_::InitEye_(vr::Hmd_Eye which_eye, Eye_ &eye) {
    eye.eye = which_eye;

    eye.texture.eType       = vr::TextureType_OpenGL;
    eye.texture.eColorSpace = vr::ColorSpace_Gamma;

    // Note that GetEyeToHeadTransform() actually returns a matrix that
    // converts from head coordinates to eye coordinates. Hmmmm.
    auto &sys = *vr::VRSystem();
    const Matrix4f head_to_eye =
        ConvertMatrix_(sys.GetEyeToHeadTransform(which_eye));

    // Extract the positional and rotational offsets.
    eye.rotation = RotationFromMatrix(head_to_eye);
    eye.offset   = Vector3f(head_to_eye * Point3f::Zero());

    // These are set once the HMD is tracked.
    eye.position    = Point3f::Zero();
    eye.orientation = Rotationf::Identity();
}

void VRContext::Impl_::InitInput_() {
    auto &vin = *vr::VRInput();

    // Set up controller data.
    auto &l_controller = controllers_[Util::EnumInt(Hand::kLeft)];
    auto &r_controller = controllers_[Util::EnumInt(Hand::kRight)];
    l_controller.hand = Hand::kLeft;
    r_controller.hand = Hand::kRight;
    l_controller.device = Event::Device::kLeftController;
    r_controller.device = Event::Device::kRightController;

    // Access HMD and controller handles.
    vin.GetInputSourceHandle("/user/head",       &headset_handle_);
    vin.GetInputSourceHandle("/user/hand/left",  &l_controller.handle);
    vin.GetInputSourceHandle("/user/hand/right", &r_controller.handle);

    // Load the actions manifest.
    LoadActions_();

    // Access the default action set.
    InitActionSet_();

    // Initialize all actions.
    InitActions_();
}

void VRContext::Impl_::LoadActions_() {
    auto &vin = *vr::VRInput();
    const auto path = FilePath::GetResourcePath("json", "actions.json");
    const auto err = vin.SetActionManifestPath(path.ToString().c_str());
    if (err != vr::VRInputError_None) {
        KLOG('v', "***Error setting manifest path: " << Util::EnumName(err));
    }
    else {
        KLOG('v', "Set manifest path to '" << path.ToString() << "'");
    }
}

void VRContext::Impl_:: InitActionSet_() {
    auto &vin = *vr::VRInput();
    const auto err = vin.GetActionSetHandle("/actions/default",
                                            &action_set_.ulActionSet);
    if (err != vr::VRInputError_None) {
        KLOG('v', "***Error getting action set: " << Util::EnumName(err));
        return;
    }
    action_set_.ulRestrictedToDevice = vr::k_ulInvalidInputValueHandle;
    action_set_.nPriority = 0;
}

void VRContext::Impl_::InitActions_() {
    auto &vin = *vr::VRInput();

    auto get_action = [&](const std::string &name,
                          vr::VRActionHandle_t &action) {
        const std::string &path = "/actions/default/in/" + name;
        const auto err = vin.GetActionHandle(path.c_str(), &action);
        if (err == vr::VRInputError_None) {
            KLOG('v', "Got action for path '" << path);
        }
        else {
            KLOG('v', "***Error getting action for path '" << path
                 << "': " << Util::EnumName(err));
        }
    };

    // Buttons.
    for (auto but: Util::EnumValues<Button_>())
        get_action(Util::EnumToWord(but), actions_.buttons[Util::EnumInt(but)]);

    // Others.
    auto &l_hand_pose = actions_.hand_poses[Util::EnumInt(Hand::kLeft)];
    auto &r_hand_pose = actions_.hand_poses[Util::EnumInt(Hand::kRight)];
    get_action("ThumbPosition", actions_.thumb_pos);
    get_action("LeftHandPose",  l_hand_pose);
    get_action("RightHandPose", r_hand_pose);
}

void VRContext::Impl_::InitEyeRendering_(Renderer &renderer, Eye_ &eye) {
    using ion::gfx::FramebufferObject;
    using ion::gfx::Image;
    using ion::gfx::ImagePtr;
    using ion::gfx::Sampler;
    using ion::gfx::SamplerPtr;
    using ion::gfx::Texture;
    using ion::gfx::TexturePtr;

    const std::string eye_str = std::string("VR ") +
        (eye.eye == vr::Eye_Left ? "L" : "R") + " Eye ";

    const auto w = window_size_[0];
    const auto h = window_size_[1];
    const int kSampleCount = 4;

    // Rendered FBO with multisampled color and depth/stencil attachments.
    auto &rendered_fbo = eye.fb_target.rendered_fbo;
    rendered_fbo.Reset(new FramebufferObject(w, h));
    rendered_fbo->SetLabel(eye_str + "Rendered FBO");
    rendered_fbo->SetColorAttachment(
        0U, FramebufferObject::Attachment::CreateMultisampled(
            Image::kRgba8888, kSampleCount));
    auto depth_stencil = FramebufferObject::Attachment::CreateMultisampled(
        Image::kRenderbufferDepth24Stencil8, kSampleCount);
    rendered_fbo->SetDepthAttachment(depth_stencil);
    rendered_fbo->SetStencilAttachment(depth_stencil);

    // Resolved FBO sampler, image, and texture.
    SamplerPtr sampler(new Sampler);
    sampler->SetMinFilter(Sampler::kLinear);
    sampler->SetMagFilter(Sampler::kLinear);
    sampler->SetWrapS(Sampler::kClampToEdge);
    sampler->SetWrapT(Sampler::kClampToEdge);
    ImagePtr resolved_image(new Image);
    resolved_image->Set(Image::kRgba8888, w, h,
                        ion::base::DataContainerPtr());
    TexturePtr resolved_tex(new Texture);
    resolved_tex->SetLabel(eye_str + "Resolved Texture");
    resolved_tex->SetSampler(sampler);
    resolved_tex->SetMaxLevel(0);
    resolved_tex->SetImage(0U, resolved_image);

    // Resolved FBO.
    auto &resolved_fbo = eye.fb_target.resolved_fbo;
    resolved_fbo.Reset(new FramebufferObject(w, h));
    resolved_fbo->SetLabel(eye_str + "Resolved FBO");
    resolved_fbo->SetColorAttachment(
        0U, FramebufferObject::Attachment(resolved_tex));
}

void VRContext::Impl_::UpdateEyes_(const Point3f &base_position) {
    auto &comp = *vr::VRCompositor();

    const uint32 kCount = vr::k_unMaxTrackedDeviceCount;
    vr::TrackedDevicePose_t poses[kCount];
    if (comp.WaitGetPoses(poses, kCount, nullptr, 0) !=
        vr::VRCompositorError_None) {
        std::cerr << "*** Unable to get poses\n";
        return;
    }

    // Get the current HMD pose data and use it to update the Eye_ data.
    const auto &hmd_pose = poses[vr::k_unTrackedDeviceIndex_Hmd];
    if (hmd_pose.bPoseIsValid) {
        const Matrix4f m = ConvertMatrix_(hmd_pose.mDeviceToAbsoluteTracking);
        const Rotationf rot = RotationFromMatrix(m);

        head_pos_ = base_position + m * Point3f::Zero();

        l_eye_.position    = head_pos_ + m * l_eye_.offset;
        r_eye_.position    = head_pos_ + m * r_eye_.offset;
        l_eye_.orientation = rot * l_eye_.rotation;
        r_eye_.orientation = rot * r_eye_.rotation;
    }
}

void VRContext::Impl_::RenderEye_(Eye_ &eye, const SG::Scene &scene,
                                  Renderer &renderer) {
    auto &sys  = *vr::VRSystem();
    auto &comp = *vr::VRCompositor();

    // Set up the viewing frustum for the eye.
    const float kNear = 0.1f;
    const float kFar  = 300.0f;
    Frustum frustum;
    float left, right, down, up;
    sys.GetProjectionRaw(eye.eye, &left, &right, &down, &up);
    frustum.SetFromTangents(left, right, down, up);
    frustum.pnear       = kNear;
    frustum.pfar        = kFar;
    frustum.viewport    = Viewport::BuildWithSize(Point2i(0, 0),
                                                  Vector2i(window_size_));
    frustum.position    = eye.position;
    frustum.orientation = eye.orientation;

    // Render.
    renderer.RenderScene(scene, frustum, &eye.fb_target);

    // Get the resolved framebuffer texture ID.
    const uint32 id = renderer.GetResolvedTextureID(eye.fb_target);
    eye.texture.handle = reinterpret_cast<void *>(id);

    // Submit the resolved texture to the VR compositor.
    comp.Submit(eye.eye, &eye.texture);
}

void VRContext::Impl_::AddButtonEvents_(Button_ but,
                                        std::vector<Event> &events) {
    auto &vin = *vr::VRInput();

    auto &action = actions_.buttons[Util::EnumInt(but)];
    ASSERT(action != vr::k_ulInvalidActionHandle);

    vr::InputDigitalActionData_t data;
    if (vin.GetDigitalActionData(action, &data, sizeof(data),
                                 vr::k_ulInvalidInputValueHandle)
        != vr::VRInputError_None)
        return;

    // Check if the button state changed. If so, add an event.
    if (data.bActive && data.bChanged) {
        Event event;

        vr::InputOriginInfo_t info;
        if (vin.GetOriginTrackedDeviceInfo(data.activeOrigin, &info,
                                           sizeof(info)) !=
            vr::VRInputError_None)
            return;

        if (info.devicePath == headset_handle_)
            event.device = Event::Device::kHeadset;
        else {
            for (int i = 0; i < 2; ++i)
                if (info.devicePath == controllers_[i].handle)
                    event.device = controllers_[i].device;
        }
        if (event.device == Event::Device::kUnknown)
            return;

        event.flags.Set(data.bState ? Event::Flag::kButtonPress :
                        Event::Flag::kButtonRelease);
        event.button = GetEventButton_(but);
        events.push_back(event);
    }
}

void VRContext::Impl_::AddHandPoseEvent_(Hand hand, std::vector<Event> &events,
                                         const Point3f &base_position) {
    auto &vin = *vr::VRInput();

    const int hand_index = Util::EnumInt(hand);
    auto &controller = controllers_[hand_index];
    auto &hand_pose  = actions_.hand_poses[hand_index];

    vr::InputPoseActionData_t data;
    const auto err = vin.GetPoseActionDataForNextFrame(
        hand_pose, vr::TrackingUniverseSeated, &data, sizeof(data),
        vr::k_ulInvalidInputValueHandle);
    const bool got_pose =
        err == vr::VRInputError_None && data.bActive && data.pose.bPoseIsValid;

    // Get the current position and orientation if there is a pose. If not,
    // leave the default values to indicate that the controller is not active.
    Point3f   pos{0, 0, 0};
    Rotationf rot;
    if (got_pose) {
        // The hand position matrix is relative to the default headset
        // position. If the headset is currently on, add the head position so
        // that the controller is viewed as relative to it. Otherwise, move it
        // so it is visible in the GLFWViewer screen window.
        const Vector3f kScreenControllerOffset(0, 1.5f, -10.5f);
        const Matrix4f m = ConvertMatrix_(data.pose.mDeviceToAbsoluteTracking);
        const Point3f offset = is_headset_on_ ? head_pos_ :
            base_position + kScreenControllerOffset;
        pos = m * Point3f::Zero() + offset;
        rot = RotationFromMatrix(m);
    }

    Event event;
    event.device = controller.device;

    event.flags.Set(Event::Flag::kPosition3D);
    event.position3D = pos;
    event.motion3D   = pos - controller.prev_position;

    event.flags.Set(Event::Flag::kOrientation);
    event.orientation = rot;

    events.push_back(event);

    controller.prev_position = pos;
}

bool VRContext::Impl_::GetButtonState_(Button_ but) {
    auto &vin = *vr::VRInput();

    auto &action = actions_.buttons[Util::EnumInt(but)];
    ASSERT(action != vr::k_ulInvalidActionHandle);

    vr::InputDigitalActionData_t data;
    const auto err = vin.GetDigitalActionData(action, &data, sizeof(data),
                                              vr::k_ulInvalidInputValueHandle);
    if (err != vr::VRInputError_None) {
        std::cerr << "XXXX Error getting data: " << Util::EnumName(err);
        return false;
    }

    return data.bActive && data.bState;
}

Event::Button VRContext::Impl_::GetEventButton_(Button_ but) {
    switch (but) {
      case Button_::kHeadsetOnHead: return Event::Button::kHeadset;
      case Button_::kPinch:         return Event::Button::kPinch;
      case Button_::kGrip:          return Event::Button::kGrip;
      case Button_::kMenu:          return Event::Button::kMenu;
      case Button_::kCenter:        return Event::Button::kCenter;
      case Button_::kLeft:          return Event::Button::kLeft;
      case Button_::kRight:         return Event::Button::kRight;
      case Button_::kUp:            return Event::Button::kUp;
      case Button_::kDown:          return Event::Button::kDown;
    }
    ASSERTM(false, "Unknown button!");
    return Event::Button::kPinch;
}

// ----------------------------------------------------------------------------
// VRContext functions.
// ----------------------------------------------------------------------------

VRContext::VRContext() : impl_(new Impl_) {
}

VRContext::~VRContext() {
}

bool VRContext::InitSystem() {
    return impl_->InitSystem();
}

void VRContext::InitRendering(Renderer &renderer) {
    impl_->InitRendering(renderer);
}

void VRContext::Render(const SG::Scene &scene, Renderer &renderer,
                       const Point3f &base_position) {
    impl_->Render(scene, renderer, base_position);
}

void VRContext::EmitEvents(std::vector<Event> &events,
                           const Point3f &base_position) {
    impl_->EmitEvents(events, base_position);
}

void VRContext::Shutdown() {
    impl_->Shutdown();
}
