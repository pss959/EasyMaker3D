#pragma once

#include <deque>
#include <vector>

#include "Base/Event.h"
#include "Base/IEmitter.h"
#include "Enums/Hand.h"
#include "Math/Types.h"

/// ScriptEmitter is a derived IEmitter class used to create events to simulate
/// mouse clicks, mouse drags and key presses in script-based applications.
class ScriptEmitter : public IEmitter {
  public:
    /// Sets modified mode for subsequent clicks and drags. It is off by
    /// default.
    void SetModifiedMode(bool is_on) { is_mod_ = is_on; }

    /// Returns true when in modified mode.
    bool IsInModifiedMode() const { return is_mod_; }

    /// Adds a mouse button press or release.
    void AddMouseButton(Event::Button button, bool is_press,
                        const Point2f &pos);

    /// Adds a controller button press or release.
    void AddControllerButton(Hand hand, Event::Button button, bool is_press,
                             const Point3f &pos, const Rotationf &rot);

    /// Adds mouse motion events between the two positions.
    void AddMouseMotion(const Point2f &pos0, const Point2f &pos1, size_t count);

    /// Adds controller motion events between the two positions and
    /// orientations.
    void AddControllerMotion(Hand hand,
                             const Point3f &pos0, const Point3f &pos1,
                             const Rotationf &rot0, const Rotationf &rot1,
                             size_t count);

    /// Returns true if a drag was started via a button press but not ended
    /// yet.
    bool IsDragging() const { return is_dragging_; }

    /// Adds a key press/release to simulate.
    void AddKey(const Str &key_string);

    /// Adds a VR headset button press or release.
    void AddHeadsetButton(bool is_press);

    /// Returns true if there are events left to process.
    bool HasPendingEvents() const { return ! events_.empty(); }

    /// Allows the current pending events to be saved and restored; this is
    /// used for pausing event processing.
    void SavePendingEvents();
    void RestorePendingEvents();

    virtual void EmitEvents(std::vector<Event> &events) override;
    virtual void FlushPendingEvents() override;

    /// Returns the position of a controller in world coordinates based on the
    /// given relative position.
    static Point3f GetWorldControllerPos(Hand hand, const Point3f &pos);

    /// Default rest position for the left controller.
    static constexpr Point3f kLeftControllerPos{-.18f, 14.06, 59.5f};

    /// Default rest position for the right controller.
    static constexpr Point3f kRightControllerPos{.18f, 14.06, 59.5f};

    /// Offset to add to left controller position for event position.
    static constexpr Vector3f kLeftControllerOffset{0, -.12f, 0};

    /// Offset to add to right controller position for event position.
    static constexpr Vector3f kRightControllerOffset{0, .12f, 0};

  private:
    /// Whether modified mode is on.
    bool               is_mod_ = false;

    /// Button to use for drag events.
    Event::Button      drag_button_ = Event::Button::kMouse1;

    /// True in the middle of a phased drag.
    bool               is_dragging_ = false;

    /// Events left to emit.
    std::deque<Event>  events_;

    /// Events saved for pausing.
    std::deque<Event>  saved_pending_events_;

    /// Set to true if the previous event was a button press. This is used to
    /// detect clicks to handle timeout correctly.
    bool               prev_was_button_press_ = false;

    /// Set to true while waiting for a click to be processed after a timeout.
    bool               waited_for_click_ = false;
};
