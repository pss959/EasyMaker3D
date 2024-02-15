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
    /// Sets the offset to use for event touch positions for the given Hand.
    void SetTouchOffset(Hand hand, const Vector3f &offset) {
        if (hand == Hand::kLeft)
            l_touch_offset_ = offset;
        else
            r_touch_offset_ = offset;
    }

    /// Sets modified mode for subsequent clicks and drags. It is off by
    /// default.
    void SetModifiedMode(bool is_on) { is_mod_ = is_on; }

    /// Returns true when in modified mode.
    bool IsInModifiedMode() const { return is_mod_; }

    /// Sets touch mode for subsequent clicks and controller motion. It is off
    /// by default.
    void SetTouchMode(bool is_on) { is_touch_ = is_on; }

    /// Returns true when in touch mode.
    bool IsInTouchMode() const { return is_touch_; }

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

  private:
    /// Left controller touch position offset for events.
    Vector3f           l_touch_offset_;
    /// Right controller touch position offset for events.
    Vector3f           r_touch_offset_;

    /// Whether modified mode is on.
    bool               is_mod_ = false;

    /// Whether touch mode is on.
    bool               is_touch_ = false;

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
