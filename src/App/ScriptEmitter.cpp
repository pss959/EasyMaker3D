#include "App/ScriptEmitter.h"

#include "Math/Linear.h"
#include "Util/Assert.h"
#include "Util/Delay.h"
#include "Util/Tuning.h"

void ScriptEmitter::AddMouseClick(const Point2f &pos) {
    Event event;
    event.is_modified_mode = is_mod_;
    event.device           = Event::Device::kMouse;
    event.button           = Event::Button::kMouse1;
    event.position2D       = pos;
    event.flags.Set(Event::Flag::kPosition2D);

    // Press.
    event.flags.Set(Event::Flag::kButtonPress);
    events_.push_back(event);

    // Release.
    event.flags.Reset(Event::Flag::kButtonPress);
    event.flags.Set(Event::Flag::kButtonRelease);
    events_.push_back(event);
}

void ScriptEmitter::AddControllerClick(const Point3f &pos, const Rotationf &rot,
                                       Event::Device device,
                                       Event::Button button) {
    const Hand hand =
        device == Event::Device::kLeftController ? Hand::kLeft : Hand::kRight;

    Event event;
    event.is_modified_mode = is_mod_;
    event.device           = device;
    event.button           = button;

    event.flags.Set(Event::Flag::kPosition3D);
    event.flags.Set(Event::Flag::kOrientation);
    event.position3D       = GetWorldControllerPos(hand, pos);
    event.orientation      = rot;

    // Press.
    event.flags.Set(Event::Flag::kButtonPress);
    events_.push_back(event);

    // Release.
    event.flags.Reset(Event::Flag::kButtonPress);
    event.flags.Set(Event::Flag::kButtonRelease);
    events_.push_back(event);
}

void ScriptEmitter::AddHoverPoint(const Point2f &pos) {
    Event event;
    event.is_modified_mode = is_mod_;
    event.device           = Event::Device::kMouse;
    event.position2D       = pos;
    event.flags.Set(Event::Flag::kPosition2D);
    events_.push_back(event);
}

void ScriptEmitter::AddDragPoint(DragPhase phase, const Point2f &pos) {
    Event event;
    event.is_modified_mode = is_mod_;
    event.device           = Event::Device::kMouse;
    event.position2D       = pos;
    event.flags.Set(Event::Flag::kPosition2D);

    if (phase == DragPhase::kStart) {
        ASSERT(! is_dragging_);
        event.flags.Set(Event::Flag::kButtonPress);
        event.button = drag_button_;
        is_dragging_ = true;
    }
    else if (phase == DragPhase::kEnd) {
        ASSERT(is_dragging_);
        event.flags.Set(Event::Flag::kButtonRelease);
        event.button = drag_button_;
        is_dragging_ = false;
    }

    events_.push_back(event);
}

void ScriptEmitter::AddDragPoints(const Point2f &pos0, const Point2f &pos1,
                                  size_t count) {
    ASSERT(! is_dragging_);
    AddDragPoint(DragPhase::kStart, pos0);
    AddIntermediateDragPoints(pos0, pos1, count);
    AddDragPoint(DragPhase::kEnd, pos1);
}

void ScriptEmitter::AddIntermediateDragPoints(const Point2f &pos0,
                                              const Point2f &pos1,
                                              size_t count) {
    const float delta = 1.f / (count + 1);
    for (size_t i = 0; i <= count; ++i) {
        const float t = (i + 1) * delta;
        AddDragPoint(DragPhase::kContinue, Lerp(t, pos0, pos1));
    }
}

void ScriptEmitter::AddKey(const Str &key_string) {
    Str error;

    Event event;
    event.device    = Event::Device::kKeyboard;
    Event::ParseKeyString(key_string, event.modifiers, event.key_name, error);
    event.key_text  = Event::BuildKeyText(event.modifiers, event.key_name);

    // Press.
    event.flags.Set(Event::Flag::kKeyPress);
    events_.push_back(event);

    // Release.
    event.flags.Reset(Event::Flag::kKeyPress);
    event.flags.Set(Event::Flag::kKeyRelease);
    events_.push_back(event);
}

void ScriptEmitter::AddControllerPos(Hand hand, const Point3f &pos,
                                     const Rotationf &rot) {
    Event event;
    event.device = hand == Hand::kLeft ?
        Event::Device::kLeftController : Event::Device::kRightController;

    event.flags.Set(Event::Flag::kPosition3D);
    event.flags.Set(Event::Flag::kOrientation);
    event.position3D  = GetWorldControllerPos(hand, pos);
    event.orientation = rot;

    events_.push_back(event);
}

void ScriptEmitter::AddControllerMotion(
    Hand hand, const Point3f &pos0, const Point3f &pos1,
    const Rotationf &rot0, const Rotationf &rot1, size_t count) {
    ASSERT(count > 0);
    const float delta = 1.f / count;
    for (size_t i = 0; i <= count; ++i) {
        const float t = i * delta;
        AddControllerPos(hand, Lerp(t, pos0, pos1),
                         Rotationf::Slerp(rot0, rot1, t));
    }
}

void ScriptEmitter::AddHeadsetButton(bool is_press) {
    Event event;
    event.is_modified_mode = is_mod_;
    event.device           = Event::Device::kHeadset;
    event.flags.Set(
        is_press ? Event::Flag::kButtonPress : Event::Flag::kButtonRelease);
    event.button = Event::Button::kHeadset;

    events_.push_back(event);
}

void ScriptEmitter::SavePendingEvents() {
    saved_pending_events_ = events_;
    events_.clear();
}

void ScriptEmitter::RestorePendingEvents() {
    events_ = saved_pending_events_;
    saved_pending_events_.clear();
}

void ScriptEmitter::EmitEvents(std::vector<Event> &events) {
    // Emit the first event, if any.
    if (! events_.empty()) {
        const Event &event = events_.front();

        // If this is the end of a click (release just after press), delay
        // until after the click timeout. Set a flag so the next time will not
        // delay.
        if (prev_was_button_press_ &&
            event.flags.Has(Event::Flag::kButtonRelease)) {
            if (! waited_for_click_) {
                Util::DelayThread(TK::kMouseClickTimeout);
                waited_for_click_ = true;
                return;
            }
            waited_for_click_ = false;
        }

        prev_was_button_press_ = event.flags.Has(Event::Flag::kButtonPress);

        events.push_back(event);
        events_.pop_front();
    }
}

void ScriptEmitter::FlushPendingEvents() {
    events_.clear();
}

Point3f ScriptEmitter::GetWorldControllerPos(Hand hand, const Point3f &pos) {
    return pos + (hand == Hand::kLeft ?
                  kLeftControllerPos  + kLeftControllerOffset :
                  kRightControllerPos + kRightControllerOffset);
}
