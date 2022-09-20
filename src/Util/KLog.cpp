#include "Util/KLog.h"

// ----------------------------------------------------------------------------
// KLOG character codes:
//
//   a:   NameManager name processing.
//   A:   SG::FileMap data storage and lookup.
//   b:   Scene graph bounds computation.
//   B:   Model mesh building and invalidation.
//   c:   Scene graph object construction and destruction.
//   d:   Mouse drag positions.
//   D:   Feedback activation and deactivation.
//   e:   Events and event handling.
//   E:   Event compression and deferring.
// + f:   File reading/parsing [Should be set before parsing scene file!].
//   F:   Interactive Pane focus and activation.
//   g:   (GUI) Board and Panel opening and closing.
//   h:   MainHandler state changes.
//   i:   Intersection testing in the scene.
//   I:   Ion setup for SG nodes.
//   j:   ActionManager action processing.
//   k:   Clicks on objects.
//   K:   VirtualKeyboard state changes.
//   m:   Changes to matrices in SG Nodes.
//   M:   Model structure and visibility changes.
//   n:   Initial notification trigger.
//   N:   All notifications.
//   o:   Notification observer changes.
//   p:   Pane sizing.
// + P:   Object parsing.
//   q:   Pane size notification.
//   r:   Ion registries.
//   R:   Rendering.
// + s:   Parser name scoping and resolution.
//   S:   Selection changes.
//   t:   Threads for delayed execution.
//   T:   Tool initialization, selection, and attachment.
//   u:   Ion uniform processing.
//   v:   VR system status.
//   V:   VR input bindings.
//   w:   Session reading and writing.
//   x:   Command execution, undo, redo.
//   y:   Model status changes.
//   z:   Font loading.
//
// Special characters:
//   !:   Disables all logging. <Alt>! in the app toggles this.
//   *:   Enables all characters. Use with caution!.
//
// Codes tagged with a '+' are better set up before parsing the scene file
// by using the '--klog' option.
// ----------------------------------------------------------------------------

/// DummyStream_ does not do any output. It is used when a message key is
/// not active.
class DummyStream_ : public std::ostream {};

static DummyStream_ s_dummy_stream_;

std::string KLogger::key_string_;
size_t      KLogger::render_count_ = 0;

bool KLogger::HasKeyCharacter(char key) {
    return key_string_.find_first_of(key) != std::string::npos;
}

void KLogger::ToggleLogging() {
    auto pos = key_string_.find_first_of('!');
    if (pos != std::string::npos)
        key_string_.erase(pos, 1);
    else
        key_string_ += '!';
}

KLogger::KLogger(char key) : do_print_(ShouldPrint_(key)) {
    GetStream() << '[' << key << "/" << render_count_ << "] " ;
}

std::ostream & KLogger::GetStream() {
    return do_print_ ? std::cout : s_dummy_stream_;
}

bool KLogger::ShouldPrint_(char key) {
    return (HasKeyCharacter(key) || HasKeyCharacter('*')) &&
        ! HasKeyCharacter('!');
}
