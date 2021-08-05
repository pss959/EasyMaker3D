#include "VR/OpenXRVRInput.h"

#include <string.h>

#include <iostream>
#include <vector>

#include "Util.h"

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

void OpenXRVRInput::PollInput(std::vector<Event> &events) {
    SyncActions_();

    for (int i = 0; i < 2; ++i) {
        const ControllerState_ &state = controller_state_[i];
        AddButtonEvent_(state, grip_action_,  Event::Button::kGrip, events);
        AddButtonEvent_(state, menu_action_,  Event::Button::kMenu, events);
        AddButtonEvent_(state, pinch_action_, Event::Button::kPinch, events);
    }
}

void OpenXRVRInput::SyncActions_() {
    const XrActiveActionSet active_action_set{ action_set_, XR_NULL_PATH };
    XrActionsSyncInfo sync_info{ XR_TYPE_ACTIONS_SYNC_INFO };
    sync_info.countActiveActionSets = 1;
    sync_info.activeActionSets = &active_action_set;
    CHECK_XR_(xrSyncActions(session_, &sync_info));
}

void OpenXRVRInput::AddButtonEvent_(const ControllerState_ &controller_state,
                                    XrAction action, Event::Button button,
                                    std::vector<Event> &events) {
    XrActionStateGetInfo get_info{ XR_TYPE_ACTION_STATE_GET_INFO };
    get_info.subactionPath = controller_state.path;
    get_info.action        = action;

    XrActionStateBoolean button_state{ XR_TYPE_ACTION_STATE_BOOLEAN };
    CHECK_XR_(xrGetActionStateBoolean(session_, &get_info, &button_state));
    if (button_state.changedSinceLastSync) {
        Event event;
        event.device = controller_state.hand == Hand::kLeft ?
            Event::Device::kLeftController : Event::Device::kRightController;
        event.flags.Set(button_state.currentState ? Event::Flag::kButtonPress :
                        Event::Flag::kButtonRelease);
        event.button = button;
        events.push_back(event);
    }
}

void OpenXRVRInput::InitInput_() {
    // Create an action set.
    XrActionSetCreateInfo action_set_info{ XR_TYPE_ACTION_SET_CREATE_INFO };
    strcpy(action_set_info.actionSetName,          "default");
    strcpy(action_set_info.localizedActionSetName, "Default");
    action_set_info.priority = 0;
    CHECK_XR_(xrCreateActionSet(instance_, &action_set_info, &action_set_));

    // Get the XrPath for each hand.
    CHECK_XR_(xrStringToPath(
                  instance_, "/user/hand/left",
                  &controller_state_[Util::EnumInt(Hand::kLeft)].path));
    CHECK_XR_(xrStringToPath(
                  instance_, "/user/hand/right",
                  &controller_state_[Util::EnumInt(Hand::kRight)].path));

    // Create actions.
    CreateInputAction_("grip",  grip_action_);
    CreateInputAction_("menu",  menu_action_);
    CreateInputAction_("pinch", pinch_action_);

    AddControllerBindings();
}

void OpenXRVRInput::CreateInputAction_(const char *name, XrAction &action) {
    assert(action_set_ != XR_NULL_HANDLE);

    XrPath subaction_paths[2];
    for (int i = 0; i < 2; ++i)
        subaction_paths[i] = controller_state_[i].path;

    XrActionCreateInfo info{ XR_TYPE_ACTION_CREATE_INFO };
    info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy(info.actionName, name);
        strcpy(info.localizedActionName, name);
        info.countSubactionPaths = 2;
        info.subactionPaths = subaction_paths;
        CHECK_XR_(xrCreateAction(action_set_, &info, &action));
}

void OpenXRVRInput::AddControllerBindings() {
    std::vector<InputBinding_> bindings;

    for (int i = 0; i < 2; ++i) { // XXXX
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
    std::cerr << "XXXX Adding binding for path '" << path_name << "'\n";
    InputBinding_ binding;
    binding.action    = action;
    binding.path_name = path_name;
    CHECK_XR_(xrStringToPath(instance_, path_name.c_str(), &binding.path));
    return binding;
}

