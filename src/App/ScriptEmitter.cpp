#include "App/ScriptEmitter.h"

#include "Math/Linear.h"
#include "Util/Assert.h"
#include "Util/Delay.h"
#include "Util/Tuning.h"

void ScriptEmitter::AddMouseButton(Event::Button button, bool is_press,
                                   const Point2f &pos) {
    ASSERT(is_dragging_ != is_press);
    is_dragging_ = is_press;

    Event event;
    event.is_modified_mode = is_mod_;
    event.device           = Event::Device::kMouse;
    event.button           = button;
    event.position2D       = pos;

    event.flags.Set(is_press ? Event::Flag::kButtonPress :
                    Event::Flag::kButtonRelease);
    event.flags.Set(Event::Flag::kPosition2D);

    events_.push_back(event);
}

void ScriptEmitter::AddControllerButton(Hand hand, Event::Button button,
                                        bool is_press,
                                        const Point3f &pos,
                                        const Rotationf &rot) {
    ASSERT(is_dragging_ != is_press);
    is_dragging_ = is_press;

    Event event;
    event.is_modified_mode = is_mod_;
    event.device           = Event::GetControllerForHand(hand);
    event.button           = button;
    event.position3D       = pos;
    event.orientation      = rot;

    event.flags.Set(is_press ? Event::Flag::kButtonPress :
                    Event::Flag::kButtonRelease);
    event.flags.Set(Event::Flag::kPosition3D);
    event.flags.Set(Event::Flag::kOrientation);
    if (is_touch_)
        event.flags.Set(Event::Flag::kTouch);

    events_.push_back(event);
}

void ScriptEmitter::AddMouseMotion(const Point2f &pos0, const Point2f &pos1,
                                   size_t count) {
    ASSERT(count >= 1U);
    Event event;
    event.is_modified_mode = is_mod_;
    event.device = Event::Device::kMouse;
    event.flags.Set(Event::Flag::kPosition2D);

    const float delta = 1.f / count;
    for (size_t i = 0; i < count; ++i) {
        const float t = (i + 1) * delta;
        event.position2D = Lerp(t, pos0, pos1);
        events_.push_back(event);
    }
}

void ScriptEmitter::AddControllerMotion(
    Hand hand, const Point3f &pos0, const Point3f &pos1,
    const Rotationf &rot0, const Rotationf &rot1, size_t count) {
    ASSERT(count >= 1U);
    Event event;
    event.is_modified_mode = is_mod_;
    event.device = Event::GetControllerForHand(hand);
    event.flags.Set(Event::Flag::kPosition3D);
    event.flags.Set(Event::Flag::kOrientation);
    if (is_touch_)
        event.flags.Set(Event::Flag::kTouch);

    const float delta = 1.f / count;
    for (size_t i = 0; i < count; ++i) {
        const float t = (i + 1) * delta;
        event.position3D  = Lerp(t, pos0, pos1);
        event.orientation = Rotationf::Slerp(rot0, rot1, t);
        if (is_touch_) {
            event.touch_position3D =
                event.position3D + event.orientation *
                (hand == Hand::kLeft ? l_touch_offset_ : r_touch_offset_);
        }
        events_.push_back(event);
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
