#include "VR/OpenXRVRInput.h"

#include <string.h>

#include <iostream>
#include <vector>

#include "Event.h"
#include "Hand.h"
#include "Util.h"

OpenXRVRInput::OpenXRVRInput(XrInstance instance, XrSession session) :
    instance_(instance), session_(session) {
    InitInput_();
}

OpenXRVRInput::~OpenXRVRInput() {
    /* XXXX
    for (auto hand : {Side::LEFT, Side::RIGHT}) {
        xrDestroySpace(m_input.handSpace[hand]);
    } */
    xrDestroyActionSet(input_state_.action_set);
}

void OpenXRVRInput::PollInput(std::vector<Event> &events) {
    // Sync actions.
    const XrActiveActionSet active_action_set{
        input_state_.action_set, XR_NULL_PATH };
    XrActionsSyncInfo sync_info{ XR_TYPE_ACTIONS_SYNC_INFO };
    sync_info.countActiveActionSets = 1;
    sync_info.activeActionSets = &active_action_set;
    CHECK_XR_(xrSyncActions(session_, &sync_info));

    for (int i = 0; i < 2; ++i) {
        XrActionStateGetInfo get_info{ XR_TYPE_ACTION_STATE_GET_INFO };
        get_info.subactionPath = input_state_.controller_state[i].path;

        get_info.action = input_state_.pinch_action;
        XrActionStateBoolean pinch_state{ XR_TYPE_ACTION_STATE_BOOLEAN };
        CHECK_XR_(xrGetActionStateBoolean(session_, &get_info, &pinch_state));
        if (pinch_state.changedSinceLastSync) {
            Event event;
            event.device = i == 0 ? Event::Device::kLeftController :
                Event::Device::kRightController;
            event.flags.Set(pinch_state.currentState ? Event::Flag::kButtonPress :
                            Event::Flag::kButtonRelease);
            event.button = Event::Button::kPinch;
            // XXXX Add position and orientation...
            events.push_back(event);
        }

        get_info.action = input_state_.menu_action;
        XrActionStateBoolean menu_state{ XR_TYPE_ACTION_STATE_BOOLEAN };
        CHECK_XR_(xrGetActionStateBoolean(session_, &get_info, &menu_state));
        if (menu_state.changedSinceLastSync) {
            Event event;
            event.device = i == 0 ? Event::Device::kLeftController :
                Event::Device::kRightController;
            event.flags.Set(menu_state.currentState ? Event::Flag::kButtonPress :
                            Event::Flag::kButtonRelease);
            event.button = Event::Button::kMenu;
            // XXXX Add position and orientation...
            events.push_back(event);
        }

        get_info.action = input_state_.grip_action;
        XrActionStateBoolean grip_state{ XR_TYPE_ACTION_STATE_BOOLEAN };
        CHECK_XR_(xrGetActionStateBoolean(session_, &get_info, &grip_state));
        if (grip_state.changedSinceLastSync) {
            Event event;
            event.device = i == 0 ? Event::Device::kLeftController :
                Event::Device::kRightController;
            event.flags.Set(grip_state.currentState ? Event::Flag::kButtonPress :
                            Event::Flag::kButtonRelease);
            event.button = Event::Button::kGrip;
            // XXXX Add position and orientation...
            events.push_back(event);
        }
    }
}

void OpenXRVRInput::InitInput_() {
    // Create an action set.
    XrActionSetCreateInfo action_set_info{ XR_TYPE_ACTION_SET_CREATE_INFO };
    strcpy(action_set_info.actionSetName,          "default");
    strcpy(action_set_info.localizedActionSetName, "Default");
    action_set_info.priority = 0;
    CHECK_XR_(xrCreateActionSet(instance_, &action_set_info,
                                &input_state_.action_set));

    // Access the controller state for each hand for convenience.
    InputState_::ControllerState_ *hand_state[2]{
        &input_state_.controller_state[0],
        &input_state_.controller_state[1] };

    // Get the XrPath for each hand.
    CHECK_XR_(xrStringToPath(instance_, "/user/hand/left",
                             &hand_state[Util::EnumInt(Hand::kLeft)]->path));
    CHECK_XR_(xrStringToPath(instance_, "/user/hand/right",
                             &hand_state[Util::EnumInt(Hand::kRight)]->path));

    // Create actions.
    CreateInputAction_("pinch", input_state_.pinch_action);
    CreateInputAction_("grip",  input_state_.grip_action);
    CreateInputAction_("menu",  input_state_.menu_action);

    AddControllerBindings();
}

void OpenXRVRInput::CreateInputAction_(const char *name, XrAction &action) {
    assert(input_state_.action_set != XR_NULL_HANDLE);

    XrPath subaction_paths[2];
    for (int i = 0; i < 2; ++i)
        subaction_paths[i] = input_state_.controller_state[i].path;

    XrActionCreateInfo info{ XR_TYPE_ACTION_CREATE_INFO };
    info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    strcpy(info.actionName, name);
        strcpy(info.localizedActionName, name);
        info.countSubactionPaths = 2;
        info.subactionPaths = subaction_paths;
        CHECK_XR_(xrCreateAction(input_state_.action_set, &info, &action));
}

void OpenXRVRInput::AddControllerBindings() {
    std::vector<InputBinding_> bindings;

    for (int i = 0; i < 2; ++i) { // XXXX
        const char *hand_name =
            i == Util::EnumInt(Hand::kLeft) ? "left" : "right";
        std::string base_path =
            std::string("/user/hand/") + hand_name + "/input/";

        bindings.push_back(BuildInputBinding_(base_path + "squeeze/click",
                                              input_state_.grip_action));
        bindings.push_back(BuildInputBinding_(base_path + "menu/click",
                                              input_state_.menu_action));
        bindings.push_back(BuildInputBinding_(base_path + "trigger/click",
                                              input_state_.pinch_action));
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
    attach_info.actionSets = &input_state_.action_set;
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

