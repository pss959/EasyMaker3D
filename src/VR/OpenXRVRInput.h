#pragma once

#include "VR/OpenXRVRBase.h"

#include <string>
#include <vector>

class Event;

//! The OpenXRVRInput class manages input (controller and headset) for the
//! OpenXRVR class.
class OpenXRVRInput : public OpenXRVRBase {
  public:
    //! The constructor is passed the XrInstance and XrSession to use.
    OpenXRVRInput(XrInstance instance, XrSession session);
    virtual ~OpenXRVRInput();

    //! Polls the OpenXR input system, possibly adding Event instances to the
    //! given vector.
    void PollInput(std::vector<Event> &events);

  private:
    //! Stores the current state of all inputs.
    struct InputState_ {
        //! Stores the input state for each controller.
        struct ControllerState_ {
            XrPath   path;
            XrSpace  space;
            float    scale = 1.0f;
            XrBool32 active;
        };
        XrActionSet      action_set   = XR_NULL_HANDLE;
        XrAction         pinch_action = XR_NULL_HANDLE;
        XrAction         grip_action  = XR_NULL_HANDLE;
        XrAction         menu_action  = XR_NULL_HANDLE;
        ControllerState_ controller_state[2];
    };

    //! This struct represents the binding of an input (by path string) to an
    //! action in the InputState_.
    struct InputBinding_ {
        XrAction    action;
        std::string path_name;
        XrPath      path;
    };

    XrInstance  instance_        = nullptr;
    XrSession   session_         = XR_NULL_HANDLE;
    InputState_ input_state_;

    void          InitInput_();
    void          CreateInputAction_(const char *name, XrAction &action);
    void          AddControllerBindings();
    InputBinding_ BuildInputBinding_(const std::string &path_name,
                                     XrAction action);
};
