#pragma once

#include "VR/OpenXRVRBase.h"

#include <string>
#include <vector>

#include <ion/math/rotation.h>
#include <ion/math/vector.h>

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
    //! given vector. The reference space for the headset is supplied along
    //! with the time used to access OpenXR device info.
    void AddEvents(std::vector<Event> &events,
                   const ion::math::Point3f &base_view_position,
                   XrSpace reference_space, XrTime time);

  private:
    //! Stores the input state per controller.
    struct ControllerState_ {
        //! \name Fields set at initialization.
        //!@{
        Hand           hand;    //!< Hand the controller corresponds to.
        Event::Device  device;  //! Event::Device code for the controller.
        XrPath         path;    //!< Sub-action path for the controller.
        XrSpace        space;   //!< Pose space for the controller.
        //!@}

        //! \name Fields set per frame.
        //!@{
        bool                 is_active;      //!< True if controller is active.
        ion::math::Point3f   position;       //!< 3D position.
        ion::math::Rotationf orientation;    //!< Orientation rotation.
        ion::math::Point3f   prev_position;  //!< Previous 3D position.
        //!@}
    };

    //! This struct represents the binding of an input (by path string) to an
    //! action in the InputState_.
    struct InputBinding_ {
        std::string path_name;
        XrPath      path;
        XrAction    action;
    };

    // TODO: Document All Of This.

    XrInstance  instance_     = XR_NULL_HANDLE;
    XrSession   session_      = XR_NULL_HANDLE;
    XrActionSet action_set_   = XR_NULL_HANDLE;

    XrAction    grip_action_  = XR_NULL_HANDLE;
    XrAction    menu_action_  = XR_NULL_HANDLE;
    XrAction    pinch_action_ = XR_NULL_HANDLE;
    XrAction    pose_action_  = XR_NULL_HANDLE;

    ControllerState_ controller_state_[2]; //!< One instance per hand.

    // Initialization.
    void          InitInput_();
    void          CreateInputAction_(const char *name, XrActionType type,
                                     XrAction &action);
    void          CreatePoseSpace_(ControllerState_ &state);
    void          AddControllerBindings();
    InputBinding_ BuildInputBinding_(const std::string &path_name,
                                     XrAction action);
    // Event emitting.
    void SyncActions_();
    void UpdateControllerState_(ControllerState_ &state,
                                const ion::math::Point3f &base_view_position,
                                XrSpace reference_space, XrTime time);
    void AddButtonEvent_(const ControllerState_ &state,
                         XrAction action, Event::Button button,
                         std::vector<Event> &events);
    void AddPoseEvent_(const ControllerState_ &state,
                       std::vector<Event> &events);
};
