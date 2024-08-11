//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Enums/Hand.h"
#include "Math/Types.h"
#include "Util/Flags.h"

/// The Event struct represents some sort of an input event. The flags indicate
/// what type of information is present in the rest of the struct.
///
/// \ingroup Base
struct Event {
  public:
    /// Devices.
    enum class Device {
        kKeyboard,
        kMouse,
        kLeftController,
        kRightController,
        kHeadset,
        kUnknown,
    };

    /// Flags indicating what information is stored.
    enum class Flag : uint32_t {
        kButtonPress    = (1 << 0),  ///< Press of a button.
        kButtonRelease  = (1 << 1),  ///< Release of a button.
        kKeyPress       = (1 << 2),  ///< Press of a key.
        kKeyRelease     = (1 << 3),  ///< Release of a key.
        kPosition1D     = (1 << 4),  ///< 1D position.
        kPosition2D     = (1 << 5),  ///< 2D position.
        kPosition3D     = (1 << 6),  ///< 3D position and motion.
        kOrientation    = (1 << 7),  ///< 3D orientation relative to axes.
        kTouch          = (1 << 8),  ///< VR controller in touch mode.
        kExit           = (1 << 9),  ///< Something causing application exit.
    };

    /// Button specifier. Mouse buttons are numbered starting from the index
    /// finger side.
    enum class Button {
        kMouse1,  ///< Mouse button on index finger side.
        kMouse2,  ///< Mouse button in middle.
        kMouse3,  ///< Mouse button on other side.
        kPinch,   ///< Controller pinch button.
        kGrip,    ///< Controller grip button.
        kMenu,    ///< Controller menu button.
        kCenter,  ///< Controller center trackpad/joystick button
        kLeft,    ///< Controller left trackpad/joystick button
        kRight,   ///< Controller right trackpad/joystick button
        kUp,      ///< Controller up trackpad/joystick button
        kDown,    ///< Controller down trackpad/joystick button
        kHeadset, ///< Headset on/off "button".
        kOther,   ///< Some other type of button.
        kNone,    ///< Used when no button action occurred.
    };

    /// Flags indicating which Keyboard modifiers are active.
    enum class ModifierKey : uint32_t {
        kShift   = (1 << 0),  ///< Shift key.
        kControl = (1 << 1),  ///< Control key.
        kAlt     = (1 << 2),  ///< Alt key.
    };

    /// Alias for a collection of ModifierKey flags.
    using Modifiers = Util::Flags<ModifierKey>;

    /// Serial number for the event. Primarily for logging and debugging.
    size_t            serial = 0;

    /// Device that generated the event.
    Device            device = Device::kUnknown;

    /// Flags indicating what information the event holds.
    Util::Flags<Flag> flags;

    /// Button specifier (kButtonPress or kButtonRelease).
    Button            button = Button::kNone;

    /// Identifying string for a key press or release. This does not include
    /// modifiers.
    Str               key_name;

    /// Text string for a key press or release, including modifiers. This will
    /// be empty for some keys.
    Str               key_text;

    /// Current keyboard modifiers for a key press or release.
    Modifiers         modifiers;

    /// Relative 1D position change for a scroll wheel or other 1D valuator.
    float             position1D = 0;

    /// 2D position for a mouse, trackpad, or similar device. Normalized to
    // (0,1) in both dimensions, where (0,0) is the lower-left corner.
    Point2f           position2D{0, 0};

    /// 3D position for a controller.
    Point3f           position3D{0, 0, 0};

    /// If kTouch is set, this is the touch affordance position.
    Point3f           touch_position3D{0, 0, 0};

    /// Change in 3D position for a controller.
    Vector3f          motion3D{0, 0, 0};

    /// 3D controller orientation. This is always relative to the default
    /// coordinate axes, with +X to the right, +Y forward, and +Z up. The
    /// orientation will be an identity rotation if the controller is not
    /// active.
    Rotationf         orientation;

    /// Flag indicating whether modified mode is active.
    bool              is_modified_mode = false;

    /// Convenience that returns true if the given Button represents a trackpad
    /// button.
    static bool IsTrackpadButton(Button button);

    /// Convenience function that returns an Event::Device for the controller
    /// that goes with the given Hand.
    static Device GetControllerForHand(Hand hand);

    /// Convenience function that builds a string representing a key press or
    /// release with all modifiers in the event. (Example: "Shift-Ctrl-a".)
    Str GetKeyString() const;

    /// Convenience function that does the reverse of BuildKeyString(); it
    /// parses the given string and sets the \p modifiers and \p key_name from
    /// it. Returns false and sets the error message on error.
    static bool ParseKeyString(const Str &key_string, Modifiers &modifiers,
                               Str &key_name, Str &error);

    /// Convenience function that builds a string representing the #key_text
    /// for the given modifiers and key name.
    static Str BuildKeyText(const Modifiers &modifiers, const Str &key_name);

    /// Convenience function that builds a string representing a key press or
    /// release with the given modifiers and key code. (Example:
    /// "Shift-Ctrl-a".)
    static Str BuildKeyString(const Modifiers &modifiers, const Str &key_name);

    /// Convenience function that builds a string representing a Controller
    /// button press or release. (Example: "L:Center" or "R:Menu".)
    Str GetControllerButtonString() const;

    /// Converts to a string to help with debugging.
    Str ToString() const;
};
