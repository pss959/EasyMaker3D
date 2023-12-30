#include "App/ScriptedApp.h"

#include <ion/math/rangeutils.h>
#include <ion/math/transformutils.h>

#include "App/Args.h"
#include "App/ScriptEmitter.h"
#include "Debug/Shortcuts.h"
#include "Items/Controller.h"
#include "Items/Settings.h"
#include "Managers/AnimationManager.h"
#include "Managers/PanelManager.h"
#include "Managers/SceneContext.h"
#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Panels/FilePanel.h"
#include "SG/CoordConv.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/FilePathList.h"
#include "Util/KLog.h"
#include "Util/Tuning.h"

namespace {

// ----------------------------------------------------------------------------
// MockFilePathList_ class.
// ----------------------------------------------------------------------------

/// Derived FilePathList class that simulates a file system so that
/// documentation images are consistent and predictable.
class MockFilePathList_ : public FilePathList {
    /// Redefines this to simulate files.
    virtual void GetContents(StrVec &subdirs, StrVec &files,
                             const Str &extension,
                             bool include_hidden) const override;
    virtual bool IsValidDirectory(const FilePath &path) const {
        const Str fn = path.GetFileName();
        return fn.starts_with("Dir") || fn == "stl" || fn == "maker";
    }
    virtual bool IsExistingFile(const FilePath &path) const {
        return true;
    }
};

void MockFilePathList_::GetContents(StrVec &subdirs, StrVec &files,
                                    const Str &extension,
                                    bool include_hidden) const {
    const FilePath dir = GetCurrent();
    // Special case for dummy path used in doc.
    if (dir.ToString() == "/projects/maker/stl/") {
        files.push_back("Airplane.stl");
        files.push_back("Boat.stl");
        files.push_back("Car.stl");
    }
    else {
        subdirs.push_back("Dir0");
        subdirs.push_back("Dir1");
        files.push_back("FileA" + extension);
        files.push_back("FileB" + extension);
        files.push_back("FileC" + extension);
        files.push_back("FileD" + extension);
        files.push_back("FileE" + extension);
    }
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// ScriptedApp functions.
// ----------------------------------------------------------------------------

void ScriptedApp::InitOptions(const Args &args) {
    // KLogging for debugging.
    KLogger::SetKeyString(args.GetString("--klog"));

    options_.do_ion_remote      = true;
    options_.enable_vr          = true;   // So controllers work properly.
    options_.fullscreen         = args.GetBool("--fullscreen");
    options_.remain             = args.GetBool("--remain");
    options_.report             = args.GetBool("--report");

    // Window size. Note that this must have the same aspect ratio as
    // fullscreen.
    int size_n = args.GetAsInt("--size", 1);
    if (size_n <= 0)
        size_n = 1;
    options_.window_size.Set(1024 / size_n, 552 / size_n);
}

bool ScriptedApp::ProcessScript(const FilePath &script_path) {
    // Read the script.
    if (! script_.ReadScript(script_path))
        return false;

    if (! Application::Init(options_))
        return false;

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
    const FilePath path("PublicDoc/settings/Settings" + TK::kDataFileExtension);
    if (! LoadSettings(path))
        return false;

    // Use the MockFilePathList_ for the FilePanel and ImportToolPanel.
    const auto set_mock = [&](const Str &panel_name){
        const auto &panel_mgr = *GetContext().panel_manager;
        auto panel = panel_mgr.GetTypedPanel<FilePanel>(panel_name);
        panel->SetFilePathList(new MockFilePathList_);
    };
    set_mock("FilePanel");
    set_mock("ImportToolPanel");

    try {
        MainLoop();
    }
    catch (AssertException &ex) {
        std::cerr << "*** Caught Assertion failure: " << ex.what() << "\n";
        std::cerr << "*** STACK:\n";
        for (const auto &s: ex.GetStackTrace())
            std::cerr << "  " << s << "\n";
        return -1;
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception: " << ex.what() << "\n";
        return -1;
    }

    return true;
}

bool ScriptedApp::ProcessFrame(size_t render_count, bool force_poll) {
    // If paused, just render, waiting for events.
    if (is_paused_)
        return Application::ProcessFrame(render_count, false);

    const size_t instr_count         = script_.GetInstructions().size();
    const bool events_pending        = emitter_->HasPendingEvents();
    const bool are_more_instructions = cur_instruction_ < instr_count;
    bool keep_going;
    bool was_stopped = false;

    // Let the derived class know the frame is starting.
    BeginFrame();

    // Let the base class check for exit. Force it to poll for events if there
    // are instructions left to process, there are pending emitter events, or
    // if the window is supposed to go away; don't want to wait for an event to
    // come along.
    const bool should_poll =
        are_more_instructions || events_pending || ! options_.remain;
    if (! Application::ProcessFrame(render_count, should_poll)) {
        keep_going = false;
    }

    // If there are events pending, do not process more instructions before
    // they are handled.
    else if (events_pending) {
        keep_going = true;
    }

    else if (are_more_instructions) {
        const auto &instr = *script_.GetInstructions()[cur_instruction_];
        const size_t instr_count = script_.GetInstructions().size();
        if (options_.report)
            std::cout << "  Processing " << instr.name
                      << " (instruction " << (cur_instruction_ + 1)
                      << " of " << instr_count << ") on line "
                      << instr.line_number << "\n";
        if (instr.name == "stop") {
            keep_going  = false;
            was_stopped = true;
            cur_instruction_ = instr_count;
            emitter_->FlushPendingEvents();
        }
        else {
            keep_going = ProcessInstruction(instr);
            ++cur_instruction_;
        }
    }

    // There are no pending events and no more instructions. If not currently
    // animating, processing is done.
    else if (! GetContext().animation_manager->IsAnimating()) {
        was_stopped = true;
    }

    // Let the derived class know the frame is done.
    EndFrame();

    // If processing is done, let the derived class know and stop ignoring
    // mouse events from GLFWViewer.
    if (was_stopped) {
        InstructionsDone();
        EnableMouseMotionEvents(true);
        keep_going = options_.remain;
    }

    return keep_going;
}

bool ScriptedApp::PauseOrUnpause() {
    is_paused_ = ! is_paused_;
    std::cerr << "*** " << (is_paused_ ? "PAUSED" : "UNPAUSED") << "\n";
    if (is_paused_) {
        emitter_->SavePendingEvents();
        EnableMouseMotionEvents(true);
    }
    else {
        emitter_->RestorePendingEvents();
        EnableMouseMotionEvents(false);
    }
    return is_paused_;
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

bool ScriptedApp::GetNodeRect(const Str &path_string, float margin,
                              Range2f &rect) {
    // Get a path to the node.
    const auto path = GetNodePath(path_string);
    if (path.empty()) {
        std::cerr << "*** No node named '" << path_string << "' found\n";
        return false;
    }

    // Compute the object-to-world matrix.
    const Matrix4f ctm = SG::CoordConv(path).GetObjectToRootMatrix();

    // For each object-coordinate bounds corner point, transform it to world
    // coordinates and project it onto the image plane to get the extents of
    // the rectangle. Note that this gives a much tighter rectangle in many
    // cases compared to transforming the bounds.
    Point3f corners[8];
    path.back()->GetBounds().GetCorners(corners);
    rect.MakeEmpty();
    const auto &frustum = *GetContext().scene_context->frustum;
    for (const auto &corner: corners)
        rect.ExtendByPoint(frustum.ProjectToImageRect(ctm * corner));

    // Add the margin.
    const Vector2f margin_vec(margin, margin);
    rect.Set(rect.GetMinPoint() - margin_vec, rect.GetMaxPoint() + margin_vec);

    // Clamp to (0,1) in both dimensions.
    rect = RangeIntersection(rect, Range2f(Point2f(0, 0), Point2f(1, 1)));
    return true;
}

SG::NodePath ScriptedApp::GetNodePath(const Str &path_string) {
    SG::NodePtr root = GetContext().scene_context->scene->GetRootNode();
    SG::NodePath path;  // Path from scene root to target object.

    // Find the node in the scene. Note that the path_string may be compound
    // ("A/B/C").
    if (path_string.contains('/')) {
        const auto parts = ion::base::SplitString(path_string, "/");
        ASSERT(parts.size() > 1U);
        for (const auto &part: parts) {
            auto sub_path = SG::FindNodePathUnderNode(root, part, true);
            if (sub_path.empty())  // Not found.
                return SG::NodePath();
            else if (path.empty())
                path = sub_path;
            else
                path = SG::NodePath::Stitch(path, sub_path);
            root = path.back();
        }
    }
    else {
        path = SG::FindNodePathUnderNode(root, path_string, true);
    }
    return path;
}
