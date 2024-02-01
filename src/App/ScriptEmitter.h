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
    /// Phase of a drag operation.
    enum class DragPhase { kStart, kContinue, kEnd };

    using KModifiers = Util::Flags<Event::ModifierKey>;

    /// Sets modified mode for subsequent clicks and drags. It is off by
    /// default.
    void SetModifiedMode(bool is_on) { is_mod_ = is_on; }

    /// Returns true when in modified mode.
    bool IsInModifiedMode() const { return is_mod_; }

    /// Adds a click to emit.
    void AddClick(const Point2f &pos);

    /// Adds an event to simulate a mouse hover at a given position.
    void AddHoverPoint(const Point2f &pos);

    /// Sets a mouse button to use for subsequent drags.
    void SetDragButton(const Event::Button &button) { drag_button_ = button; }

    /// Adds a point for a single drag phase to emit.
    void AddDragPoint(DragPhase phase, const Point2f &pos);

    /// Returns true if a drag was started with DragPhase::kStart but not ended
    /// yet with DragPhase::kEnd.
    bool IsDragging() const { return is_dragging_; }

    /// Adds points for a drag from \p pos0 to \p pos1 with \p count
    /// intermediate points to emit.
    void AddDragPoints(const Point2f &pos0, const Point2f &pos1, size_t count);

    /// Adds \p count intermediate points to emit for a drag from \p pos0 to \p
    /// pos1.
    void AddIntermediateDragPoints(const Point2f &pos0, const Point2f &pos1,
                                   size_t count);

    /// Adds a key press/release to simulate.
    void AddKey(const Str &key_string);

    /// Adds a controller position.
    void AddControllerPos(Hand hand, const Point3f &pos, const Rotationf &rot);

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
