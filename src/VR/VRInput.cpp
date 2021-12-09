#include "VR/VRInput.h"

#include <iostream>

#include "Util/Enum.h"
#include "VR/VRContext.h"
#include "VR/VRStructs.h"

// ----------------------------------------------------------------------------
// OpenXRVRInput implementation.
// ----------------------------------------------------------------------------

VRInput::VRInput(VRContext &context) : context_(context) {
    VRBase::SetInstance(GetInstance_());
    InitInput_();
}

VRInput::~VRInput() {
    for (int i = 0; i < 2; ++i) {
        if (controller_state_[i].space)
            xrDestroySpace(controller_state_[i].space);
    }
    xrDestroyActionSet(action_set_);
}

void VRInput::EmitEvents(std::vector<Event> &events,
                         const Point3f &base_position) {
    try {
        const bool was_headset_on = is_headset_on_;
        if (! PollEvents_()) {
            // Add exit event
            Event event;
            event.flags.Set(Event::Flag::kExit);
            events.push_back(event);
        }
        else {
            if (context_.GetTime())
                AddEvents_(events, base_position, context_.GetReferenceSpace(),
                           context_.GetTime());
        }
        if (is_headset_on_ != was_headset_on)
            AddHeadsetEvent_(is_headset_on_, events);
    }
    catch (VRException_ &ex) {
        ReportException_(ex);
    }
}

void VRInput::EndSession() {
    // Initiate the exit process.
    CHECK_XR_(xrRequestExitSession(GetSession_()));

    // Wait for it to work.
    while (PollEvents_())
        ;
}

void VRInput::InitInput_() {
    ASSERT_(GetInstance_() != XR_NULL_HANDLE);
    ASSERT_(GetSession_()  != XR_NULL_HANDLE);

    // Create an action set.
    XrActionSetCreateInfo action_set_info = VRS::BuildActionSetCreateInfo();
    strcpy(action_set_info.actionSetName,          "default");
    strcpy(action_set_info.localizedActionSetName, "Default");
    action_set_info.priority = 0;
    CHECK_XR_(xrCreateActionSet(GetInstance_(),
                                &action_set_info, &action_set_));

    ControllerState_ &lstate = controller_state_[Util::EnumInt(Hand::kLeft)];
    ControllerState_ &rstate = controller_state_[Util::EnumInt(Hand::kRight)];

    lstate.hand = Hand::kLeft;
    rstate.hand = Hand::kRight;
    lstate.device = Event::Device::kLeftController;
    rstate.device = Event::Device::kRightController;

    // Get the XrPath for each hand.
    CHECK_XR_(xrStringToPath(GetInstance_(), "/user/hand/left",  &lstate.path));
    CHECK_XR_(xrStringToPath(GetInstance_(), "/user/hand/right", &rstate.path));

    // Create actions.
    CreateInputAction_("grip",  XR_ACTION_TYPE_BOOLEAN_INPUT, grip_action_);
    CreateInputAction_("menu",  XR_ACTION_TYPE_BOOLEAN_INPUT, menu_action_);
    CreateInputAction_("pinch", XR_ACTION_TYPE_BOOLEAN_INPUT, pinch_action_);
    CreateInputAction_("pose",  XR_ACTION_TYPE_POSE_INPUT,    pose_action_);

    // Create a space for each controller.
    CreatePoseSpace_(lstate);
    CreatePoseSpace_(rstate);

    // Set up the bindings for all supported controllers.
    AddControllerBindings();
}

void VRInput::CreateInputAction_(const char *name, XrActionType type,
                                 XrAction &action) {
    ASSERT_(action_set_ != XR_NULL_HANDLE);

    XrPath subaction_paths[2];
    for (int i = 0; i < 2; ++i)
        subaction_paths[i] = controller_state_[i].path;

    XrActionCreateInfo info = VRS::BuildActionCreateInfo();
    info.type = XR_TYPE_ACTION_CREATE_INFO;
    info.actionType = type;
    strcpy(info.actionName, name);
    strcpy(info.localizedActionName, name);
    info.countSubactionPaths = 2;
    info.subactionPaths = subaction_paths;
    CHECK_XR_(xrCreateAction(action_set_, &info, &action));
}

void VRInput::CreatePoseSpace_(ControllerState_ &state) {
    XrActionSpaceCreateInfo space_info = VRS::BuildActionSpaceCreateInfo();
    space_info.action = pose_action_;
    space_info.poseInActionSpace.orientation.w = 1.f;
    space_info.subactionPath = state.path;
    CHECK_XR_(xrCreateActionSpace(GetSession_(), &space_info, &state.space));
}

void VRInput::AddControllerBindings() {
    std::vector<InputBinding_> bindings;

    for (int i = 0; i < 2; ++i) {
        const char *hand_name =
            i == Util::EnumInt(Hand::kLeft) ? "left" : "right";
        std::string base_path =
            std::string("/user/hand/") + hand_name + "/input/";

        bindings.push_back(BuildInputBinding_(base_path + "squeeze/click",
                                              grip_action_));
        bindings.push_back(BuildInputBinding_(base_path + "menu/click",
                                              menu_action_));
        bindings.push_back(BuildInputBinding_(base_path + "trigger/click",
                                              pinch_action_));
        bindings.push_back(BuildInputBinding_(base_path + "grip/pose",
                                              pose_action_));
    }

    // Set up bindings for Vive Controller. XXXX
    XrPath vive_path;
    CHECK_XR_(xrStringToPath(GetInstance_(),
                             "/interaction_profiles/htc/vive_controller",
                             &vive_path));
    std::vector<XrActionSuggestedBinding> vive_bindings;
    for (const InputBinding_ &binding: bindings) {
        vive_bindings.push_back(XrActionSuggestedBinding{
                binding.action, binding.path});
    }
    XrInteractionProfileSuggestedBinding suggested_bindings =
        VRS::BuildInteractionProfileSuggestedBinding();
    suggested_bindings.interactionProfile     = vive_path;
    suggested_bindings.suggestedBindings      = vive_bindings.data();
    suggested_bindings.countSuggestedBindings =
        static_cast<uint32_t>(vive_bindings.size());
    CHECK_XR_(xrSuggestInteractionProfileBindings(GetInstance_(),
                                                  &suggested_bindings));

    XrSessionActionSetsAttachInfo attach_info =
        VRS::BuildSessionActionSetsAttachInfo();
    attach_info.countActionSets = 1;
    attach_info.actionSets      = &action_set_;
    CHECK_XR_(xrAttachSessionActionSets(GetSession_(), &attach_info));
}

VRInput::InputBinding_ VRInput::BuildInputBinding_(
    const std::string &path_name, XrAction action) {
    InputBinding_ binding;
    binding.action    = action;
    binding.path_name = path_name;
    CHECK_XR_(xrStringToPath(GetInstance_(), path_name.c_str(), &binding.path));
    return binding;
}

bool VRInput::PollEvents_() {
    // Process all pending messages.
    bool keep_going = true;
    XrEventDataBuffer event;
    while (keep_going && GetNextEvent_(event)) {
        switch (event.type) {
          case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
            ReportDisaster_("OpenXR instance loss pending");
            break;
          case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
            keep_going = ProcessSessionStateChange_(
                CAST_(const XrEventDataSessionStateChanged &, event));
            break;

          case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
#if XXXX
              XrPath path;
              CHECK_XR_(xrStringToPath(GetInstance_(),
                                       "/user/hand/right", &path));
              XrInteractionProfileState ps;
              ps.type = XR_TYPE_INTERACTION_PROFILE_STATE;
              ps.next = nullptr;
              CHECK_XR_(xrGetCurrentInteractionProfile(GetSession_(),
                                                       path, &ps));
              std::cerr << "XXXX   New path = "
                        << PathToString(ps.interactionProfile) << "\n";
              // XXXX Eventually use this to detect controller type?
#endif
              break;
          }

          case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
          default:
            // TODO: See if these are required to handle.
            std::cout << "*** Ignoring VR event type "
                      << Util::EnumName(event.type) << "\n";
            break;
        }
    }
    return keep_going;
}

bool VRInput::GetNextEvent_(XrEventDataBuffer &event) {
    event.type = XR_TYPE_EVENT_DATA_BUFFER;
    return xrPollEvent(GetInstance_(), &event) == XR_SUCCESS;
}

bool VRInput::ProcessSessionStateChange_(
    const XrEventDataSessionStateChanged &event) {
    XrSessionState session_state = event.state;

    if (event.session != XR_NULL_HANDLE && event.session != GetSession_()) {
        ReportDisaster_("State change for unknown session");
        return true;
    }

    bool keep_going = true;
    switch (session_state) {
      case XR_SESSION_STATE_READY: {
          XrSessionBeginInfo info = VRS::BuildSessionBeginInfo();
          info.primaryViewConfigurationType = context_.GetViewType();
          CHECK_XR_(xrBeginSession(GetSession_(), &info));
          break;
      }

      case XR_SESSION_STATE_STOPPING:
        CHECK_XR_(xrEndSession(GetSession_()));
        keep_going = false;
        break;

      case XR_SESSION_STATE_EXITING:
      case XR_SESSION_STATE_LOSS_PENDING:
        keep_going = false;
        break;

      default:
        break;
    }

    // Detect changes to headset state.
    is_headset_on_ = session_state == XR_SESSION_STATE_FOCUSED;

    return keep_going;
}

void VRInput::AddEvents_(std::vector<Event> &events,
                         const Point3f &base_position,
                         XrSpace reference_space, XrTime time) {
    SyncActions_();

    for (int i = 0; i < 2; ++i) {
        ControllerState_ &state = controller_state_[i];

        // Get the current controller state and pose.
        UpdateControllerState_(state, base_position, reference_space, time);

        // Add button events only if the controller is active.
        if (state.is_active) {
            AddButtonEvent_(state, grip_action_,  Event::Button::kGrip,
                            events);
            AddButtonEvent_(state, menu_action_,  Event::Button::kMenu,
                            events);
            AddButtonEvent_(state, pinch_action_, Event::Button::kPinch,
                            events);
        }

        // Always add an event with the position and orientation. The
        // orientation rotation will be identity if the controller is not
        // active.
        AddPoseEvent_(state, events);
        state.prev_position = state.position;
    }
}

void VRInput::SyncActions_() {
    XrActiveActionSet active_action_set = VRS::BuildActiveActionSet();
    active_action_set.actionSet = action_set_;

    XrActionsSyncInfo sync_info = VRS::BuildActionsSyncInfo();
    sync_info.countActiveActionSets = 1;
    sync_info.activeActionSets      = &active_action_set;
    CHECK_XR_(xrSyncActions(GetSession_(), &sync_info));
}

void VRInput::UpdateControllerState_(ControllerState_ &state,
                                     const Point3f &base_position,
                                     XrSpace reference_space, XrTime time) {
    XrActionStateGetInfo get_info = VRS::BuildActionStateGetInfo();
    get_info.subactionPath = state.path;
    get_info.action        = pose_action_;
    XrActionStatePose pose_state;
    pose_state.type = XR_TYPE_ACTION_STATE_POSE;
    CHECK_XR_(xrGetActionStatePose(GetSession_(), &get_info, &pose_state));

    state.position    = base_position;
    state.orientation = Rotationf::Identity();

    state.is_active = pose_state.isActive;
    if (state.is_active) {
        XrSpaceLocation loc = VRS::BuildSpaceLocation();
        CHECK_XR_(xrLocateSpace(state.space, reference_space, time, &loc));
        if (loc.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
            state.position += ToVector3f(loc.pose.position);
        if (loc.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)
            state.orientation = ToRotationf(loc.pose.orientation);
    }
}

void VRInput::AddButtonEvent_(const ControllerState_ &state, XrAction action,
                              Event::Button button,
                              std::vector<Event> &events) {
    XrActionStateGetInfo get_info = VRS::BuildActionStateGetInfo();
    get_info.subactionPath = state.path;
    get_info.action        = action;

    XrActionStateBoolean button_state = VRS::BuildActionStateBoolean();
    CHECK_XR_(xrGetActionStateBoolean(GetSession_(), &get_info, &button_state));
    if (button_state.changedSinceLastSync) {
        Event event;
        event.device = state.device;
        event.flags.Set(button_state.currentState ? Event::Flag::kButtonPress :
                        Event::Flag::kButtonRelease);
        event.button = button;
        events.push_back(event);
    }
}

void VRInput::AddPoseEvent_(const ControllerState_ &state,
                            std::vector<Event> &events) {
    Event event;
    event.device = state.device;

    event.flags.Set(Event::Flag::kPosition3D);
    event.position3D = state.position;
    event.motion3D   = state.position - state.prev_position;

    event.flags.Set(Event::Flag::kOrientation);
    event.orientation = state.orientation;

    events.push_back(event);
}

void VRInput::AddHeadsetEvent_(bool is_on, std::vector<Event> &events) {
    Event event;
    event.device = Event::Device::kHeadset;
    event.flags.Set(is_on ? Event::Flag::kButtonPress :
                    Event::Flag::kButtonRelease);
    event.button = Event::Button::kHeadset;
    events.push_back(event);
}

XrInstance VRInput::GetInstance_() const {
    return context_.GetInstance();
}

XrSession VRInput::GetSession_() const {
    return context_.GetSession();
}

std::string VRInput::PathToString(const XrPath &path) {
    if (path == XR_NULL_PATH)
        return "<NULL>";

    char     str[XR_MAX_PATH_LENGTH];
    uint32_t size;
    CHECK_XR_(xrPathToString(GetInstance_(), path, XR_MAX_PATH_LENGTH,
                             &size, str));
    return str;
}
