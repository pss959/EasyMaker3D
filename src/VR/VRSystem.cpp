//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "VR/VRSystem.h"

#ifdef ION_PLATFORM_WINDOWS
#  include "openvr-fixed.h"
#else
#  define __stdcall
#  include "openvr-fixed.h"
#  undef  __stdcall
#endif

#include <ion/gfx/framebufferobject.h>
#include <ion/gfx/image.h>
#include <ion/math/transformutils.h>

#include "Base/Event.h"
#include "Base/FBTarget.h"
#include "Math/Frustum.h"
#include "Math/Linear.h"
#include "SG/Node.h"
#include "SG/VRCamera.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/FilePath.h"
#include "Util/KLog.h"
#include "Util/Tuning.h"
#include "VR/VRModelLoader.h"
#include "Viewers/IRenderer.h"

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
// VRSystem::Impl_ class.
// ----------------------------------------------------------------------------

/// This class does most of the work for the VRSystem.
class VRSystem::Impl_ {
  public:
    bool Startup();
    void Shutdown();
    bool LoadControllerModel(Hand hand, Controller::CustomModel &model);
    void SetControllers(const ControllerPtr &l_controller,
                        const ControllerPtr &r_controller);
    void InitRendering(IRenderer &renderer);
    void SetCamera(const SG::VRCameraPtr &cam) { camera_ = cam; }
    void Render(const SG::Scene &scene, IRenderer &renderer);
    void EmitEvents(std::vector<Event> &events);
    bool IsHeadSetOn() const { return is_headset_on_; }

  private:
    /// Tracked VR controller buttons. Note that the headset on/off button is
    /// handled separately.
    enum class Button_ {
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
        FBTargetPtr   fb_target;    ///< For passing framebuffers to IRenderer.
    };

    /// OpenVR action handles and other data per Hand.
    struct HandData_ {
        vr::VRActionHandle_t buttons[Util::EnumCount<Button_>()];
        vr::VRActionHandle_t thumb_pos;
        vr::VRActionHandle_t pose;
        vr::VRActionHandle_t vibration;

        // These are used to work around the fact that OpenVR sometimes reports
        // multiple trackpad button presses (such as Left+Down) for the same
        // click over a short interval.
        bool     is_trackpad_button_pressed = false;
        Button_  active_trackpad_button;
    };

    /// Information about each controller.
    struct Controller_ {
        Hand          hand;        ///< Corresponding Hand.
        ControllerPtr controller;  ///< Corresponding Controller object.
        Event::Device device;      ///< Corresponding device.

        vr::VRInputValueHandle_t handle = vr::k_ulInvalidInputValueHandle;

        /// This saves the position of the controller at the previous frame so
        /// that the motion can be computed.
        Point3f prev_position{0, 0, 0};
    };

    SG::VRCameraPtr camera_;             ///< SG::Camera used for VR.
    Point3f         camera_position_;    ///< Cached position of the camera.

    // Rendering.
    Vector2ui window_size_;
    Eye_      l_eye_;
    Eye_      r_eye_;

    // HMD.
    vr::TrackedDeviceIndex_t hmd_index_;      ///< HMD device index.
    Point3f                  head_pos_;       ///< Current HMD position.
    float                    head_y_offset_;  ///< HMD height offset of head.
    bool             is_headset_on_ = false;  ///< True if HMD is being worn.

    /// This is set to true if there is a binding for the headset button,
    /// meaning that it can be tracked via normal means. Otherwise, a different
    /// method is used.
    bool             has_headset_button_ = false;

    /// Saves the name of the device (HMD) manufacturer. This is used to work
    /// around OpenVR device-dependent tracking weirdness when computing
    /// relative controller positions.
    Str              manufacturer_;

    // Devices.
    vr::VRInputValueHandle_t headset_handle_ = vr::k_ulInvalidInputValueHandle;
    Controller_              controllers_[2];  // Indexed by Hand enum.

    // Input.
    vr::VRActiveActionSet_t action_set_;
    vr::VRActionHandle_t    headset_action_;
    HandData_               hand_data_[2];  ///< Indexed by Hand.

    bool CheckForHMD_();
    bool InitVR_();
    void InitEye_(vr::Hmd_Eye which_eye, Eye_ &eye);
    bool InitInput_();
    bool LoadActions_();
    void InitActionSet_();
    void InitActions_();
    vr::VRActionHandle_t GetHandAction_(Hand hand, const Str &name,
                                        bool is_input);
    vr::VRActionHandle_t GetAction_(const Str &path);
    bool WereBindingsLoadedSuccessfully_();
    void InitEyeRendering_(IRenderer &renderer, Eye_ &eye);
    void UpdateEyes_();
    void RenderEye_(Eye_ &eye, const SG::Scene &scene, IRenderer &renderer);
    void VibrateController_(Hand hand, float duration);
    void AddButtonEvent_(Hand hand, Button_ but, std::vector<Event> &events);
    void AddHandPoseToEvent_(Hand hand, Event &event);
    void AddThumbPosToEvent_(Hand hand, Event &event);
    Event CreateEventForHand_(Hand hand);
    bool GetButtonState_(const vr::VRActionHandle_t &action);
    bool HasHeadsetButton_();
    bool IsHeadsetOn_();
    static Event::Button GetEventButton_(Button_ but);

#if ENABLE_DEBUG_FEATURES
    void ReportAllBindings_();
    void ReportBindings_(const Str &path, const vr::VRActionHandle_t &action);
#endif
};

// ----------------------------------------------------------------------------
// VRSystem::Impl_ functions.
// ----------------------------------------------------------------------------

bool VRSystem::Impl_::Startup() {
    KLOG('v', "Starting VR system");

    // Init VR without requiring hardware to see if there is an HMD present. If
    // that passes, init for real to make sure everything works as expected.
    if (! CheckForHMD_() || ! InitVR_())
        return false;

    // Set up the eyes.
    InitEye_(vr::Eye_Left,  l_eye_);
    InitEye_(vr::Eye_Right, r_eye_);

    // Set up input.
    return InitInput_();
}

void VRSystem::Impl_::Shutdown() {
    KLOG('v', "Shutting down VR");
    vr::VR_Shutdown();
}

bool VRSystem::Impl_::LoadControllerModel(Hand hand,
                                          Controller::CustomModel &model) {
    auto &controller = controllers_[Util::EnumInt(hand)];
    const auto handle = controller.handle;
    return VRModelLoader::LoadControllerModel(handle, hand, model);
}

void VRSystem::Impl_::SetControllers(const ControllerPtr &l_controller,
                                     const ControllerPtr &r_controller) {
    controllers_[Util::EnumInt(Hand::kLeft)].controller  = l_controller;
    controllers_[Util::EnumInt(Hand::kRight)].controller = r_controller;

    // Set up vibration.
    l_controller->SetVibrateFunc(
        [&](float duration){ VibrateController_(Hand::kLeft,  duration); });
    r_controller->SetVibrateFunc(
        [&](float duration){ VibrateController_(Hand::kRight, duration); });
}

void VRSystem::Impl_::InitRendering(IRenderer &renderer) {
    KLOG('v', "Initializing VR rendering");

    // Set up VR for rendering.
    auto &sys = *vr::VRSystem();

    sys.GetRecommendedRenderTargetSize(&window_size_[0], &window_size_[1]);
    KLOG('v', "VR window size is "
         << window_size_[0] << " x " << window_size_[1]);

    InitEyeRendering_(renderer, l_eye_);
    InitEyeRendering_(renderer, r_eye_);
}

void VRSystem::Impl_::Render(const SG::Scene &scene, IRenderer &renderer) {
    // Update the cached camera position.
    ASSERT(camera_);
    camera_position_ = camera_->GetCurrentPosition();

    // Make sure the eyes are positioned and rotated correctly.
    UpdateEyes_();

    // Render each eye into textures.
    RenderEye_(l_eye_, scene, renderer);
    RenderEye_(r_eye_, scene, renderer);

    // Apparently an OpenVR render has to occur before bindings are accessible.
    if (renderer.GetFrameCount() == 0) {
        // Determine if the headset button is bound. If not, use the HMD device
        // activity level to detect headset changes.
        has_headset_button_ = HasHeadsetButton_();
#if ENABLE_DEBUG_FEATURES
        ReportAllBindings_();
#endif
    }
}

void VRSystem::Impl_::EmitEvents(std::vector<Event> &events) {
    // Update the cached camera position for pose computation.
    ASSERT(camera_);
    camera_position_ = camera_->GetCurrentPosition();

    // Controller instances must have been set.
    ASSERT(controllers_[0].controller);
    ASSERT(controllers_[1].controller);

    auto &vin = *vr::VRInput();

    if (vin.UpdateActionState(&action_set_, sizeof(action_set_), 1) !=
        vr::VRInputError_None) {
        std::cerr << "*** Error updating action state\n";
    }

    // Determine if the headset is currently on. This needs to be set before
    // the hand poses can be processed properly. If the state changed, create a
    // button event for it.
    const bool was_headset_on = is_headset_on_;
    is_headset_on_ = IsHeadsetOn_();
    if (is_headset_on_ != was_headset_on) {
        Event event;
        event.device = Event::Device::kHeadset;
        event.button = Event::Button::kHeadset;
        event.flags.Set(is_headset_on_ ? Event::Flag::kButtonPress :
                        Event::Flag::kButtonRelease);
        events.push_back(event);
    }

    for (auto hand: Util::EnumValues<Hand>()) {
        size_t event_count = events.size();

        // Check for input button changes. Add an event for each change.
        for (auto but: Util::EnumValues<Button_>())
            AddButtonEvent_(hand, but, events);

        // If there are no button events, add one to set the hand pose.
        if (events.size() == event_count)
            events.push_back(CreateEventForHand_(hand));

        // Add hand pose and thumb position to all new events for this Hand.
        for (size_t i = event_count; i < events.size(); ++i) {
            AddHandPoseToEvent_(hand, events[i]);
            AddThumbPosToEvent_(hand, events[i]);
        }
    }
}

bool VRSystem::Impl_::CheckForHMD_() {
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

bool VRSystem::Impl_::InitVR_() {
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

void VRSystem::Impl_::InitEye_(vr::Hmd_Eye which_eye, Eye_ &eye) {
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

bool VRSystem::Impl_::InitInput_() {
    auto &sys = *vr::VRSystem();
    auto &vin = *vr::VRInput();

    // Set up controller data.
    auto &l_controller = controllers_[Util::EnumInt(Hand::kLeft)];
    auto &r_controller = controllers_[Util::EnumInt(Hand::kRight)];
    l_controller.hand = Hand::kLeft;
    r_controller.hand = Hand::kRight;
    l_controller.device = Event::Device::kLeftController;
    r_controller.device = Event::Device::kRightController;

    // Access the HMD device in case it is needed.
    const uint32_t count = sys.GetSortedTrackedDeviceIndicesOfClass(
        vr::TrackedDeviceClass_HMD, &hmd_index_, 1);

    // Access the manufacturer to work around device-dependent problems.
    char buf[1024];
    sys.GetStringTrackedDeviceProperty(
        hmd_index_, vr::Prop_ManufacturerName_String, buf, 1024);
    manufacturer_ = buf;

    if (count == 1U) {
        // Access HMD and controller handles.
        vin.GetInputSourceHandle("/user/head",       &headset_handle_);
        vin.GetInputSourceHandle("/user/hand/left",  &l_controller.handle);
        vin.GetInputSourceHandle("/user/hand/right", &r_controller.handle);
    }
    else {
        KLOG('v', "***Error: could not access HMD device");
        return false;
    }

    // Load the actions manifest.
    if (! LoadActions_())
        return false;

    // Access the default action set.
    InitActionSet_();

    // Initialize all actions.
    InitActions_();

    // Wait for bindings to be loaded.
    if (! WereBindingsLoadedSuccessfully_())
        return false;

    return true;
}

bool VRSystem::Impl_::LoadActions_() {
    auto &vin = *vr::VRInput();
    // Note: SteamVR seems to require an absolute path for this.
    const auto path =
        FilePath::GetResourcePath("json", "actions.json").GetAbsolute();
    const auto err = vin.SetActionManifestPath(path.ToString().c_str());
    if (err != vr::VRInputError_None) {
        KLOG('v', "***Error setting manifest path to '" << path.ToString()
             << "': " << Util::EnumName(err));
        return false;
    }
    else {
        KLOG('v', "Set manifest path to '" << path.ToString() << "'");
        return true;
    }
}

void VRSystem::Impl_:: InitActionSet_() {
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

void VRSystem::Impl_::InitActions_() {
    // Headset on/off button.
    headset_action_ = GetAction_("/actions/default/in/HeadsetOnHead");

    // Per-hand actions.
    for (auto hand: Util::EnumValues<Hand>()) {
        HandData_ &data = hand_data_[Util::EnumInt(hand)];

        // Buttons.
        for (auto but: Util::EnumValues<Button_>())
            data.buttons[Util::EnumInt(but)] =
                GetHandAction_(hand, Util::EnumToWord(but), true);

        // Other per-hand stuff.
        data.thumb_pos = GetHandAction_(hand, "ThumbPosition", true);
        data.pose      = GetHandAction_(hand, "Pose",          true);
        data.vibration = GetHandAction_(hand, "Haptic",        false);
    }
}

vr::VRActionHandle_t VRSystem::Impl_::GetHandAction_(
    Hand hand, const Str &name, bool is_input) {
    Str path = "/actions/default/";
    path += is_input ? "in/" : "out/";
    path += hand == Hand::kLeft ? "L" : "R";
    path += name;
    return GetAction_(path);
}

vr::VRActionHandle_t VRSystem::Impl_::GetAction_(const Str &path) {
    auto &vin = *vr::VRInput();

    vr::VRActionHandle_t action;
    const auto err = vin.GetActionHandle(path.c_str(), &action);
    if (err == vr::VRInputError_None) {
        KLOG('v', "Got action for path '" << path);
    }
    else {
        KLOG('v', "***Error getting action for path '" << path
             << "': " << Util::EnumName(err));
    }
    ASSERT(action != vr::k_ulInvalidActionHandle);

    return action;
};

bool VRSystem::Impl_::WereBindingsLoadedSuccessfully_() {
    auto &sys = *vr::VRSystem();
    vr::VREvent_t vr_event;

    // The bindings should be loaded fairly quickly.
    for (int i = 0; i < 100; ++i) {
        if (sys.PollNextEvent(&vr_event, sizeof(vr_event))) {
            const vr::EVREventType type =
                static_cast<vr::EVREventType>(vr_event.eventType);
            if (type == vr::VREvent_Input_BindingLoadSuccessful) {
                KLOG('v', "Bindings loaded successfully");
                return true;
            }
            else if (type == vr::VREvent_Input_BindingLoadFailed) {
                KLOG('v', "***Loading bindings failed");
                return false;
            }
        }
    }

    // This shouldn't happen.
    KLOG('v', "***Bindings never loaded!");
    return false;
}

void VRSystem::Impl_::InitEyeRendering_(IRenderer &renderer, Eye_ &eye) {
    const Str eye_str =
        Str("VR ") + (eye.eye == vr::Eye_Left ? "L" : "R") + " Eye ";
    eye.fb_target.reset(new FBTarget);
    eye.fb_target->Init(eye_str, window_size_, TK::kVRSampleCount);
}

void VRSystem::Impl_::UpdateEyes_() {
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

        head_pos_      = camera_position_ + m * Point3f::Zero();
        head_y_offset_ = head_pos_[1] - camera_position_[1];

        l_eye_.position    = head_pos_ + m * l_eye_.offset;
        r_eye_.position    = head_pos_ + m * r_eye_.offset;
        l_eye_.orientation = rot * l_eye_.rotation;
        r_eye_.orientation = rot * r_eye_.rotation;
    }
}

void VRSystem::Impl_::RenderEye_(Eye_ &eye, const SG::Scene &scene,
                                  IRenderer &renderer) {
    auto &sys  = *vr::VRSystem();
    auto &comp = *vr::VRCompositor();

    // Set up the viewing frustum for the eye.
    Frustum frustum;
    float left, right, down, up;
    sys.GetProjectionRaw(eye.eye, &left, &right, &down, &up);
    frustum.SetFromTangents(left, right, down, up);
    frustum.pnear       = TK::kVRNearDistance;
    frustum.pfar        = TK::kVRFarDistance;
    frustum.viewport    = Viewport::BuildWithSize(Point2ui(0, 0),
                                                  Vector2ui(window_size_));
    frustum.position    = eye.position;
    frustum.orientation = eye.orientation;

    // Make sure the eyes are at the same height as the window camera.
    frustum.position[1] -= head_y_offset_;

    // Render.
    renderer.SetFBTarget(eye.fb_target);
    renderer.RenderScene(scene, frustum);

    // Get the resolved framebuffer texture ID.
    const uint32 id = renderer.GetResolvedTextureID();
    eye.texture.handle = reinterpret_cast<void *>(id);

    // Submit the resolved texture to the VR compositor.
    comp.Submit(eye.eye, &eye.texture);
}

void VRSystem::Impl_::VibrateController_(Hand hand, float duration) {
    auto &action = hand_data_[Util::EnumInt(hand)].vibration;
    ASSERT(action != vr::k_ulInvalidActionHandle);
    vr::VRInput()->TriggerHapticVibrationAction(
        action, 0, duration, TK::kVibrationFrequency,
        TK::kVibrationAmplitude, vr::k_ulInvalidInputValueHandle);
}

void VRSystem::Impl_::AddButtonEvent_(Hand hand, Button_ but,
                                       std::vector<Event> &events) {
    auto &vin = *vr::VRInput();

    HandData_ &data = hand_data_[Util::EnumInt(hand)];
    const auto &action = data.buttons[Util::EnumInt(but)];
    ASSERT(action != vr::k_ulInvalidActionHandle);

    // Check if the button state changed. If so, add an event.
    vr::InputDigitalActionData_t ddata;
    const auto err = vin.GetDigitalActionData(
        action, &ddata, sizeof(ddata), vr::k_ulInvalidInputValueHandle);
    if (err == vr::VRInputError_None && ddata.bActive && ddata.bChanged) {
        // Avoid multiple concurrent trackpad button presses/releases.
        const bool is_press = ddata.bState;
        const auto ebutton = GetEventButton_(but);
        const bool is_trackpad_button = Event::IsTrackpadButton(ebutton);
        if (is_trackpad_button &&
            ((is_press && data.is_trackpad_button_pressed) ||
             (! is_press && but != data.active_trackpad_button)))
            return;

        Event event = CreateEventForHand_(hand);
        event.flags.Set(is_press ? Event::Flag::kButtonPress :
                        Event::Flag::kButtonRelease);
        event.button = ebutton;
        events.push_back(event);

        if (is_press && is_trackpad_button) {
            data.is_trackpad_button_pressed = true;
            data.active_trackpad_button = but;
        }
        else if (! is_press && data.is_trackpad_button_pressed &&
                 but == data.active_trackpad_button) {
            data.is_trackpad_button_pressed = false;
        }
    }
}

void VRSystem::Impl_::AddHandPoseToEvent_(Hand hand, Event &event) {
    auto &vin = *vr::VRInput();

    const int hand_index = Util::EnumInt(hand);
    auto &controller = controllers_[hand_index];
    auto &hand_pose  = hand_data_[hand_index].pose;

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
        // This matrix defines the hand position relative to the default
        // headset position.
        const Matrix4f m = ConvertMatrix_(data.pose.mDeviceToAbsoluteTracking);
        const Point3f  rel_hand_pos = m * Point3f::Zero();

        // Make the controller position relative to the camera camera position.
        pos = camera_position_ + rel_hand_pos;

        // The height of the relative position of the controller seems to be
        // correct for the Vive but needs to be adjusted for the Oculus Quest
        // 2. Who knows why or what other devices need? Not me.
        if (manufacturer_ == "Oculus") {
            // If the headset is on, make the height relative to the headset
            // height. If the headset is not on, subtract the Y offset designed
            // to make the position consistent with the VR view.
            pos[1] -= is_headset_on_ ? head_pos_[1] - camera_position_[1] :
                TK::kHeadsetOffControllerYOffset;
        }

        // Copy the rotation.
        rot = RotationFromMatrix(m);
    }

    event.flags.Set(Event::Flag::kPosition3D);
    event.position3D = pos;
    event.motion3D   = pos - controller.prev_position;

    if (controller.controller->IsInTouchMode()) {
        const auto &offset = controller.controller->GetTouchOffset();
        event.flags.Set(Event::Flag::kTouch);
        event.touch_position3D = pos + rot * offset;
    }

    event.flags.Set(Event::Flag::kOrientation);
    event.orientation = rot;

    controller.prev_position = pos;
}

void VRSystem::Impl_::AddThumbPosToEvent_(Hand hand, Event &event) {
    auto &vin = *vr::VRInput();

    vr::InputAnalogActionData_t data;
    auto &action = hand_data_[Util::EnumInt(hand)].thumb_pos;
    const auto err = vin.GetAnalogActionData(
        action, &data, sizeof(data), vr::k_ulInvalidInputValueHandle);
    if (err == vr::VRInputError_None && data.bActive) {
        event.flags.Set(Event::Flag::kPosition2D);
        event.position2D.Set(data.x, data.y);
    }
}

Event VRSystem::Impl_::CreateEventForHand_(Hand hand) {
    Event event;
    event.device = Event::GetControllerForHand(hand);
    return event;
}

bool VRSystem::Impl_::GetButtonState_(const vr::VRActionHandle_t &action) {
    ASSERT(action != vr::k_ulInvalidActionHandle);
    auto &vin = *vr::VRInput();

    vr::InputDigitalActionData_t data;
    const auto err = vin.GetDigitalActionData(
        action, &data, sizeof(data), vr::k_ulInvalidInputValueHandle);
    return err == vr::VRInputError_None && data.bActive && data.bState;
}

bool VRSystem::Impl_::HasHeadsetButton_() {
    auto &vin = *vr::VRInput();
    vr::InputBindingInfo_t info[20];
    uint32_t               count;
    const auto err = vin.GetActionBindingInfo(headset_action_, info,
                                              sizeof(info[0]), 20, &count);
    return err == vr::VRInputError_None && count > 0;
}

bool VRSystem::Impl_::IsHeadsetOn_() {
    bool is_on = false;
    if (has_headset_button_) {
        // If the proximity sensor button is bound, get its state.
        is_on = GetButtonState_(headset_action_);
    }
    else {
        // Otherwise, check the HMD activity level.
        auto &sys = *vr::VRSystem();
        const auto level = sys.GetTrackedDeviceActivityLevel(hmd_index_);
        is_on = level == vr::k_EDeviceActivityLevel_UserInteraction;
    }
    return is_on;
}

Event::Button VRSystem::Impl_::GetEventButton_(Button_ but) {
    switch (but) {
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

#if ENABLE_DEBUG_FEATURES
void VRSystem::Impl_::ReportAllBindings_() {
    if (KLogger::HasKeyCharacter('V')) {
        KLOG('V', "VR Device manufacturer: " << manufacturer_);

        ReportBindings_("/actions/default/in/HeadsetOnHead", headset_action_);

        const auto &lh = hand_data_[Util::EnumInt(Hand::kLeft)];
        const auto &rh = hand_data_[Util::EnumInt(Hand::kRight)];
        for (auto but: Util::EnumValues<Button_>()) {
            ReportBindings_("/actions/default/in/L" + Util::EnumToWord(but),
                            lh.buttons[Util::EnumInt(but)]);
            ReportBindings_("/actions/default/in/R" + Util::EnumToWord(but),
                            rh.buttons[Util::EnumInt(but)]);
        }
    }
}

void VRSystem::Impl_::ReportBindings_(const Str &path,
                                       const vr::VRActionHandle_t &action) {
    auto &vin = *vr::VRInput();

    vr::InputBindingInfo_t info[20];
    uint32_t               count;
    const auto err =
        vin.GetActionBindingInfo(action, info, sizeof(info[0]), 20, &count);
    if (err != vr::VRInputError_None) {
        KLOG('V', "*** Error accessing binding info for '" << path << "': "
             << Util::EnumName(err));
    }
    else {
        KLOG('V', "Binding info for '" << path
             << "'; count = " << count << ":");
        for (uint32_t i = 0; i < count; ++i) {
            const auto &bi = info[i];
            KLOG('V', " Binding " << i << ":");
            KLOG('V', "  Device path = '" << bi.rchDevicePathName  << "'");
            KLOG('V', "  Input path  = '" << bi.rchInputPathName   << "'");
            KLOG('V', "  Mode        = '" << bi.rchModeName        << "'");
            KLOG('V', "  Slot        = '" << bi.rchSlotName        << "'");
            KLOG('V', "  Source type = '" << bi.rchInputSourceType << "'");
        }
    }
}
#endif

// ----------------------------------------------------------------------------
// VRSystem functions.
// ----------------------------------------------------------------------------

VRSystem::VRSystem() : impl_(new Impl_) {
}

VRSystem::~VRSystem() {
}

bool VRSystem::Startup() {
    return impl_->Startup();
}

void VRSystem::Shutdown() {
    impl_->Shutdown();
}

bool VRSystem::LoadControllerModel(Hand hand, Controller::CustomModel &model) {
    return impl_->LoadControllerModel(hand, model);
}

void VRSystem::SetControllers(const ControllerPtr &l_controller,
                               const ControllerPtr &r_controller) {
    impl_->SetControllers(l_controller, r_controller);
}

void VRSystem::InitRendering(IRenderer &renderer) {
    impl_->InitRendering(renderer);
}

void VRSystem::SetCamera(const SG::VRCameraPtr &cam) {
    impl_->SetCamera(cam);
}

void VRSystem::Render(const SG::Scene &scene, IRenderer &renderer) {
    impl_->Render(scene, renderer);
}

void VRSystem::EmitEvents(std::vector<Event> &events) {
    impl_->EmitEvents(events);
}

bool VRSystem::IsHeadSetOn() const {
    return impl_->IsHeadSetOn();
}
