#pragma once

#include "VR/OpenXRVRBase.h"

#include <string>
#include <vector>

#include "Event.h"
#include "Hand.h"

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
    //! Stores the input state per controller.
    struct ControllerState_ {
        Hand     hand;    //!< Which hand the controller corresponds to.
        XrPath   path;    //!< Sub-action path for the controller.
        XrSpace  space;   //!< XXXX
        XrBool32 active;  //!< Whether the controller is active.
    };

    //! This struct represents the binding of an input (by path string) to an
    //! action in the InputState_.
    struct InputBinding_ {
        std::string path_name;
        XrPath      path;
        XrAction    action;
    };

    XrInstance  instance_     = XR_NULL_HANDLE;
    XrSession   session_      = XR_NULL_HANDLE;
    XrActionSet action_set_   = XR_NULL_HANDLE;

    XrAction    pinch_action_ = XR_NULL_HANDLE;
    XrAction    grip_action_  = XR_NULL_HANDLE;
    XrAction    menu_action_  = XR_NULL_HANDLE;

    ControllerState_ controller_state_[2]; //!< One instance per hand.

    void          InitInput_();
    void          CreateInputAction_(const char *name, XrAction &action);
    void          AddControllerBindings();
    InputBinding_ BuildInputBinding_(const std::string &path_name,
                                     XrAction action);
    void          SyncActions_();
    void          AddButtonEvent_(const ControllerState_ &controller_state,
                                  XrAction action, Event::Button button,
                                  std::vector<Event> &events);
    void          AddButtonEvent_(XrPath path, XrAction action,
                                  Event::Button button,
                                  std::vector<Event> &events);
};
