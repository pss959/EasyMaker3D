#pragma once

#include <string>

#include <ion/math/rotation.h>
#include <ion/math/vector.h>

#include "Flags.h"

//! The Event struct represents some sort of an input event. The flags indicate
//! what type of information is present in the rest of the struct.
struct Event {
  public:
    //! Devices.
    enum class Device {
        kKeyboard,
        kMouse,
        kLeftController,
        kRightController,
        kHeadset,
        kUnknown,
    };

    //! Flags indicating what information is stored.
    enum class Flag : uint32_t {
        kButtonPress    = (1 << 0),  //!< Press of a button.
        kButtonRelease  = (1 << 1),  //!< Release of a button.
        kKeyPress       = (1 << 2),  //!< Press of a key.
        kKeyRelease     = (1 << 3),  //!< Release of a key.
        kPosition1D     = (1 << 4),  //!< 1D position.
        kPosition2D     = (1 << 5),  //!< 2D position.
        kPosition3D     = (1 << 6),  //!< 3D position.
        kOrientation    = (1 << 7),  //!< 3D orientation relative to axes.
        kExit           = (1 << 8),  //!< Something causing application exit.
    };

    //! Button specifier. Mouse buttons are numbered starting from the index
    //! finger side.
    enum class Button {
        kMouse1,  //!< Mouse button on index finger side.
        kMouse2,  //!< Mouse button in middle.
        kMouse3,  //!< Mouse button on other side.
        kPinch,   //!< Controller pinch button.
        kGrip,    //!< Controller grip button.
        kMenu,    //!< Controller menu button.
        kCenter,  //!< Controller center trackpad/joystick button
        kLeft,    //!< Controller left trackpad/joystick button
        kRight,   //!< Controller right trackpad/joystick button
        kUp,      //!< Controller up trackpad/joystick button
        kDown,    //!< Controller down trackpad/joystick button
        kHeadset, //!< Headset on/off "button".
        kOther,   //!< Some other type of button.
        kNone,    //!< Used when no button action occurred.
    };

    //! Device that generated the event.
    Device               device = Device::kUnknown;

    //! Flags indicating what information the event holds.
    Flags<Flag>          flags;

    //! Button specifier (kButtonPress or kButtonRelease).
    Button               button = Button::kNone;

    //! Identifying string for a key press or release.
    std::string          key_string;

    //! 1D position for a thumbwheel or other 1D valuator. Normalized to (0,1).
    float                position1D = 0;

    //! 2D position for a mouse, trackpad, or similar device. Normalized to
    // (0,1) in both dimensions, where (0,0) is the lower-left corner.
    ion::math::Point2f   position2D = ion::math::Point2f::Zero();

    //! 3D position for a controller.
    ion::math::Point3f   position3D = ion::math::Point3f::Zero();

    //! 3D orientation. This is always relative to the default coordinate axes,
    //! with +X to the right, +Y forward, and +Z up.
    ion::math::Rotationf orientation;
};
