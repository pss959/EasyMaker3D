#pragma once

#include <string>

#include <ion/math/vector.h>

//! The Event struct represents some sort of an input event. The flags indicate
//! what type of information is present in the rest of the struct.
struct Event {
  public:
    //! Flags indicating what information is stored.
    enum class Flag : uint32_t {
        kButtonPress    = 0x001,  //! Press of a button.
        kButtonRelease  = 0x002,  //! Release of a button.
        kKeyPress       = 0x004,  //! Press of a key.
        kKeyRelease     = 0x008,  //! Release of a key.
        kPointerRay     = 0x010,  //! Pointer ray (world coordinates).
        kGripData       = 0x020,  //! Data to allow grip hovering and dragging.
        kValuator       = 0x040,  //! Valuator value change.
        kPosition2D     = 0x080,  //! 2D trackpad or similar position.
        kExit           = 0x100,  //! Something causing application exit.
    };

    //! Types of buttons.
    enum class ButtonType {
        //! Button that activates an action involving pointing at an
        //! interactive object. The pointerRay field indicates the pointer
        //! origin and direction.
        kPointerActivator,

        //! Button that activates a grip action. The gripData field specifies
        //! the current grip information.
        kGripActivator,

        //! Some other type of button.
        kOther,
    };

    //! Device that generated the event.
    // IDevice    device;

    //! Flags indicating what information the event holds.
    uint32_t flags = 0;

    //! Type of button (kButtonPress or kButtonRelease).
    ButtonType button_type;

    //! Identifying detail string for a button or key press or release.
    std::string detail_string;

    //! Identifying string for a valuator (Valuator).
    std::string valuatorId;

    //! Relative valuator change (Valuator).
    float      valuator_change;

    //! 2D position for trackpad or similar device. Normalized to (0,1) in both
    // dimensions.
    ion::math::Vector2f position2D;
};
