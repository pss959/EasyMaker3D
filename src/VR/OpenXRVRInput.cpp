#include "VR/OpenXRVRInput.h"

#include <string.h>

#include <iostream>
#include <vector>

#include "Util.h"

using ion::math::Point3f;
using ion::math::Rotationf;

// ----------------------------------------------------------------------------
// OpenXRVRInput implementation.
// ----------------------------------------------------------------------------

OpenXRVRInput::OpenXRVRInput(XrInstance instance, XrSession session) :
    instance_(instance), session_(session) {
    InitInput_();
}

OpenXRVRInput::~OpenXRVRInput() {
    for (int i = 0; i < 2; ++i) {
        if (controller_state_[i].space)
            xrDestroySpace(controller_state_[i].space);
    }
    xrDestroyActionSet(action_set_);
}

void OpenXRVRInput::AddEvents(std::vector<Event> &events,
                              XrSpace reference_space, XrTime time) {
    SyncActions_();

    for (int i = 0; i < 2; ++i) {
        ControllerState_ &state = controller_state_[i];

        // Get the current controller state and pose.
        UpdateControllerState_(state, reference_space, time);

        if (! state.is_active)
            continue;

        AddButtonEvent_(state, grip_action_,  Event::Button::kGrip,  events);
        AddButtonEvent_(state, menu_action_,  Event::Button::kMenu,  events);
        AddButtonEvent_(state, pinch_action_, Event::Button::kPinch, events);

        // Always add an event with the position and orientation.
        AddPoseEvent_(state, events);
    }
}

void OpenXRVRInput::SyncActions_() {
    const XrActiveActionSet active_action_set{ action_set_, XR_NULL_PATH };
    XrActionsSyncInfo sync_info{ XR_TYPE_ACTIONS_SYNC_INFO };
    sync_info.countActiveActionSets = 1;
    sync_info.activeActionSets = &active_action_set;
    CHECK_XR_(xrSyncActions(session_, &sync_info));
}

void OpenXRVRInput::InitInput_() {
    // Create an action set.
    XrActionSetCreateInfo action_set_info{ XR_TYPE_ACTION_SET_CREATE_INFO };
    strcpy(action_set_info.actionSetName,          "default");
    strcpy(action_set_info.localizedActionSetName, "Default");
    action_set_info.priority = 0;
    CHECK_XR_(xrCreateActionSet(instance_, &action_set_info, &action_set_));

    ControllerState_ &lstate = controller_state_[Util::EnumInt(Hand::kLeft)];
    ControllerState_ &rstate = controller_state_[Util::EnumInt(Hand::kRight)];

    lstate.hand = Hand::kLeft;
    rstate.hand = Hand::kRight;
    lstate.device = Event::Device::kLeftController;
    rstate.device = Event::Device::kRightController;

    // Get the XrPath for each hand.
    CHECK_XR_(xrStringToPath(instance_, "/user/hand/left",  &lstate.path));
    CHECK_XR_(xrStringToPath(instance_, "/user/hand/right", &rstate.path));

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

void OpenXRVRInput::CreateInputAction_(const char *name, XrActionType type,
                                       XrAction &action) {
    assert(action_set_ != XR_NULL_HANDLE);

    XrPath subaction_paths[2];
    for (int i = 0; i < 2; ++i)
        subaction_paths[i] = controller_state_[i].path;

    XrActionCreateInfo info{ XR_TYPE_ACTION_CREATE_INFO };
    info.actionType = type;
    strcpy(info.actionName, name);
        strcpy(info.localizedActionName, name);
        info.countSubactionPaths = 2;
        info.subactionPaths = subaction_paths;
        CHECK_XR_(xrCreateAction(action_set_, &info, &action));
}

void OpenXRVRInput::CreatePoseSpace_(ControllerState_ &state) {
    XrActionSpaceCreateInfo space_info{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
    space_info.action = pose_action_;
    space_info.poseInActionSpace.orientation.w = 1.f;
    space_info.subactionPath = state.path;
    CHECK_XR_(xrCreateActionSpace(session_, &space_info, &state.space));
}

void OpenXRVRInput::AddControllerBindings() {
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

    // Set up bindings for Vive Controller.
    XrPath vive_path;
    CHECK_XR_(xrStringToPath(instance_,
                             "/interaction_profiles/htc/vive_controller",
                             &vive_path));
    std::vector<XrActionSuggestedBinding> vive_bindings;
    for (const InputBinding_ &binding: bindings) {
        vive_bindings.push_back(XrActionSuggestedBinding{
                binding.action, binding.path});
    }
    XrInteractionProfileSuggestedBinding suggested_bindings{
        XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
    suggested_bindings.interactionProfile = vive_path;
    suggested_bindings.suggestedBindings = vive_bindings.data();
    suggested_bindings.countSuggestedBindings =
        static_cast<uint32_t>(vive_bindings.size());
    CHECK_XR_(xrSuggestInteractionProfileBindings(instance_,
                                                  &suggested_bindings));

    XrSessionActionSetsAttachInfo attach_info{
        XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
    attach_info.countActionSets = 1;
    attach_info.actionSets = &action_set_;
    CHECK_XR_(xrAttachSessionActionSets(session_, &attach_info));
}

OpenXRVRInput::InputBinding_ OpenXRVRInput::BuildInputBinding_(
    const std::string &path_name, XrAction action) {
    InputBinding_ binding;
    binding.action    = action;
    binding.path_name = path_name;
    CHECK_XR_(xrStringToPath(instance_, path_name.c_str(), &binding.path));
    return binding;
}

void OpenXRVRInput::UpdateControllerState_(
    ControllerState_ &state, XrSpace reference_space, XrTime time) {
    XrActionStateGetInfo get_info{ XR_TYPE_ACTION_STATE_GET_INFO };
    get_info.subactionPath = state.path;
    get_info.action        = pose_action_;
    XrActionStatePose pose_state{ XR_TYPE_ACTION_STATE_POSE };
    CHECK_XR_(xrGetActionStatePose(session_, &get_info, &pose_state));

    state.position    = Point3f::Zero();
    state.orientation = Rotationf::Identity();

    state.is_active = pose_state.isActive;
    if (state.is_active) {
        XrSpaceLocation loc{ XR_TYPE_SPACE_LOCATION };
        CHECK_XR_(xrLocateSpace(state.space, reference_space, time, &loc));
        if (loc.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
            state.position    = ToPoint3f(loc.pose.position);
        if (loc.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)
            state.orientation = ToRotationf(loc.pose.orientation);
    }
}

void OpenXRVRInput::AddButtonEvent_(const ControllerState_ &state,
                                    XrAction action, Event::Button button,
                                    std::vector<Event> &events) {
    XrActionStateGetInfo get_info{ XR_TYPE_ACTION_STATE_GET_INFO };
    get_info.subactionPath = state.path;
    get_info.action        = action;

    XrActionStateBoolean button_state{ XR_TYPE_ACTION_STATE_BOOLEAN };
    CHECK_XR_(xrGetActionStateBoolean(session_, &get_info, &button_state));
    if (button_state.changedSinceLastSync) {
        Event event;
        event.device = state.device;
        event.flags.Set(button_state.currentState ? Event::Flag::kButtonPress :
                        Event::Flag::kButtonRelease);
        event.button = button;
        events.push_back(event);
    }
}

void OpenXRVRInput::AddPoseEvent_(const ControllerState_ &state,
                                  std::vector<Event> &events) {
    Event event;
    event.device = state.device;

    event.flags.Set(Event::Flag::kPosition3D);
    event.position3D = state.position;

    event.flags.Set(Event::Flag::kOrientation);
    event.orientation = state.orientation;

    events.push_back(event);
}
