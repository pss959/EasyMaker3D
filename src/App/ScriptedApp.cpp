#include "App/ScriptedApp.h"

#include "App/ScriptEmitter.h"
#include "Debug/Shortcuts.h"
#include "Items/Controller.h"
#include "Items/Settings.h"
#include "Managers/AnimationManager.h"
#include "Managers/SceneContext.h"
#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"

bool ScriptedApp::Init(const OptionsPtr &options, const ScriptBasePtr &script) {
    ASSERT(options);
    ASSERT(script);

    if (! Application::Init(*options))
        return false;

    options_ = options;
    script_  = script;

    emitter_.reset(new ScriptEmitter);
    AddEmitter(emitter_);

    const auto &context = GetContext();

    // Turn off controllers until they are specifically added.
    context.scene_context->left_controller->SetEnabled(false);
    context.scene_context->right_controller->SetEnabled(false);

#if ENABLE_DEBUG_FEATURES
    // Make sure there is no debug text visible.
    Debug::DisplayDebugText("");
#endif

    // Ignore mouse events from GLFWViewer so they do not interfere with the
    // click and drag events.
    EnableMouseMotionEvents(false);

    // No need to ask before quitting a scripted app.
    SetAskBeforeQuitting(false);

    // Set render offsets for the controllers.
    SetControllerRenderOffsets(-ScriptEmitter::kLeftControllerOffset,
                               -ScriptEmitter::kRightControllerOffset);

    // Use default settings file so that state is deterministic.
    const FilePath path("PublicDoc/snaps/settings/Settings" +
                        TK::kDataFileExtension);
    if (! LoadSettings(path))
        return false;

    return true;
}

bool ScriptedApp::ProcessFrame(size_t render_count, bool force_poll) {
    const size_t instr_count         = script_->GetInstructions().size();
    const bool events_pending        = emitter_->HasPendingEvents();
    const bool are_more_instructions = cur_instruction_ < instr_count;
    bool keep_going;
    bool processing_done = false;

    // Let the base class check for exit. Force it to poll for events if there
    // are instructions left to process, there are pending emitter events, or
    // if the window is supposed to go away; don't want to wait for an event to
    // come along.
    const bool should_poll =
        are_more_instructions || events_pending || ! options_->remain;
    if (! Application::ProcessFrame(render_count, should_poll)) {
        keep_going = false;
    }

    // If there are events pending, do not process more instructions before
    // they are handled.
    else if (events_pending) {
        keep_going = true;
    }

    else if (are_more_instructions) {
        const auto &instr = *script_->GetInstructions()[cur_instruction_];
        if (instr.name == "stop") {
            keep_going = false;
            processing_done = true;
            cur_instruction_ = instr_count;
        }
        else {
            keep_going = ProcessInstruction_(instr);
            ++cur_instruction_;
        }
    }

    // There are no pending events and no more instructions. If not currently
    // animating, processing is done.
    else if (! GetContext().animation_manager->IsAnimating()) {
        processing_done = true;
    }

    // Let the derived class know the frame is done.
    FrameDone();

    // If processing is done, let the derived class know and stop ignoring
    // mouse events from GLFWViewer.
    if (processing_done) {
        InstructionsDone();
        EnableMouseMotionEvents(true);
        keep_going = options_->remain;
    }

    return keep_going;
}

const ScriptedApp::Options & ScriptedApp::GetOptions() const {
    ASSERT(options_);
    return *options_;
}

const ScriptBase & ScriptedApp::GetScript() const {
    ASSERT(script_);
    return *script_;
}

ScriptEmitter & ScriptedApp::GetEmitter() const {
    ASSERT(emitter_);
    return *emitter_;
}

bool ScriptedApp::LoadSettings(const FilePath &path) {
    const auto &context = GetContext();

    // Set the new path in the SettingsManager.
    if (! context.settings_manager->SetPath(path, false)) {
        std::cerr << "*** Unable to load settings from " << path.ToString()
                  << ": " << context.settings_manager->GetLoadError() << "\n";
        return false;
    }

    // Tell the SessionManager to update its previous path.
    context.session_manager->ChangePreviousPath(
        context.settings_manager->GetSettings().GetLastSessionPath());

    return true;
}

bool ScriptedApp::ProcessInstruction_(const ScriptBase::Instr &instr) {
    const size_t instr_count = script_->GetInstructions().size();
    if (options_->report)
        std::cout << "  Processing " << instr.name
                  << " (instruction " << (cur_instruction_ + 1)
                  << " of " << instr_count << ") on line "
                  << instr.line_number << "\n";

    // Let the derived class do most of the work.
    return ProcessInstruction(instr);
}
