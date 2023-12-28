#include "App/CaptureScriptApp.h"

#include <cmath>
#include <functional>
#include <ranges>

#include <ion/base/stringutils.h>
#include <ion/gfx/image.h>
#include <ion/image/conversionutils.h>

#include "App/CaptureScript.h"
#include "App/ScriptEmitter.h"
#include "Base/Event.h"
#include "Feedback/TooltipFeedback.h"
#include "Handlers/Handler.h"
#include "Managers/AnimationManager.h"
#include "Managers/EventManager.h"
#include "Managers/SceneContext.h"
#include "Managers/SessionManager.h"
#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "Models/Model.h"
#include "Parser/Parser.h"
#include "SG/CoordConv.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "App/VideoWriter.h"
#include "SG/WindowCamera.h"
#include "Util/Assert.h"
#include "Viewers/Renderer.h"
#include "Widgets/Slider1DWidget.h"

// ----------------------------------------------------------------------------
// CaptureScriptApp::CursorHandler_ class.
// ----------------------------------------------------------------------------

/// This derived Handler class is used by the CaptureScriptApp to update the
/// fake cursor when a mouse motion event is seen.
class CaptureScriptApp::CursorHandler_ : public Handler {
  public:
    using Func = std::function<void(const Point2f &)>;

    /// The constructor is passed a function to invoke when a mouse position
    /// event is handled.
    explicit CursorHandler_(const Func &func) : func_(func) {
        ASSERT(func);
    }

    /// Defines this to invoke the mouse position function.
    virtual HandleCode HandleEvent(const Event &event) override {
        if (event.flags.Has(Event::Flag::kPosition2D)) {
            func_(event.position2D);
            // Let other handlers see the event.
            return HandleCode::kHandledContinue;
        }
        return HandleCode::kNotHandled;
    }

  private:
    Func func_;
};

// ----------------------------------------------------------------------------
// CaptureScriptApp::PauseHandler_ class.
// ----------------------------------------------------------------------------

/// This derived Handler class is used by the CaptureScriptApp to handle pause
/// and unpause events.
class CaptureScriptApp::PauseHandler_ : public Handler {
  public:
    using Func = std::function<void()>;

    /// The constructor is passed a function to invoke when a pause key event
    /// is handled.
    explicit PauseHandler_(const Func &func) : func_(func) {
        ASSERT(func);
    }

    /// Defines this to invoke the mouse position function.
    virtual HandleCode HandleEvent(const Event &event) override {
        if (event.flags.Has(Event::Flag::kKeyPress) &&
            event.GetKeyString() == "Pause") {
            func_();
            // Let other handlers see the event.
            return HandleCode::kHandledContinue;
        }
        return HandleCode::kNotHandled;
    }

  private:
    Func func_;
};

// ----------------------------------------------------------------------------
// CaptureScriptApp functions.
// ----------------------------------------------------------------------------

CaptureScriptApp::CaptureScriptApp() {}

CaptureScriptApp::~CaptureScriptApp() {}

bool CaptureScriptApp::Init(const OptionsPtr &options,
                            const ScriptBasePtr &script) {
    if (! ScriptedApp::Init(options, script))
        return false;

    const auto &context = GetContext();

    ASSERT(context.scene_context);
    ASSERT(context.scene_context->scene);
    const auto &scene = *context.scene_context->scene;
    auto room = SG::FindNodeInScene(scene, "Room");

    // Parse the Capture scene data and add it to the Room.
    Parser::Parser parser;
    auto capture_root = std::dynamic_pointer_cast<SG::Node>(
        parser.ParseFile(FilePath::GetResourcePath("nodes", "Capture.emd")));
    ASSERT(capture_root);
    room->AddChild(capture_root);

    // Find the FakeCursor and position it just in front of the camera.
    cursor_ = SG::FindNodeUnderNode(*capture_root, "FakeCursor");
    MoveFakeCursorTo_(Point2f(.5f, .5f));  // In the middle.

    // Add a Handler_ to update the fake cursor when the mouse is moved. Insert
    // it at the beginning so no other handler steals the event.
    cursor_handler_.reset(
        new CursorHandler_([&](const Point2f &p){ MoveFakeCursorTo_(p); }));
    context.event_manager->InsertHandler(cursor_handler_);

    // Add a Handler_ to allow pausing when the "remain" option is specified.
    // Insert it at the beginning so no other handler steals the event.
    if (options->remain) {
        auto pause_func = [&](){
            const bool is_paused = PauseOrUnpause();
            cursor_handler_->SetEnabled(! is_paused);
        };
        pause_handler_.reset(new PauseHandler_(pause_func));
        context.event_manager->InsertHandler(pause_handler_);
    }

    // Find the highlight node.
    highlight_ = SG::FindNodeUnderNode(*capture_root, "HighlightRect");

    // Find the caption nodes.
    caption_.node = SG::FindNodeUnderNode(*capture_root,  "Caption");
    caption_.bg   = SG::FindNodeUnderNode(*caption_.node, "Background");
    caption_.text = SG::FindTypedNodeUnderNode<SG::TextNode>(*caption_.node,
                                                             "Text");

    const auto &opts = GetOptions_();

    // Set up a VideoWriter if requested.
    if (! opts.nocapture) {
        const VideoWriter::Format format =
            opts.format == "rgbmp4" ? VideoWriter::Format::kRGBMP4 :
            opts.format == "yuvmp4" ? VideoWriter::Format::kYUVMP4 :
            VideoWriter::Format::kWEBM;
        video_writer_.reset(new VideoWriter(format));

        // Set up the output path.
        const FilePath &script_path = GetScript().GetPath();
        FilePath video_path("PublicDoc/docs/extra/videos/" +
                            script_path.GetFileName());
        video_path.ReplaceExtension("." + video_writer_->GetExtension());

        // Initialize the VideoWriter.
        video_writer_->Init(video_path, GetWindowSize(), opts.fps);
    }

    // New Models should be animated when created.
    Model::EnablePlacementAnimation(true);

    // Use a constant time increment per frame for animation so the animations
    // are not subject to inconsistent frame times due to capture.
    context.animation_manager->SetFrameIncrement(1.f / opts.fps);

    // Set a huge duration (20 seconds) so long presses do not occur. Video
    // capture is sometimes slow enough to make a press seem very long.
    SetLongPressDuration(20);

    // Disable tooltips by default - they can appear if frame grabbing is very
    // slow.
    TooltipFeedback::SetDelay(0);

    // Fake export from the SessionManager, since the path is likely bogus.
    context.session_manager->SetFakeExport(true);

    // When the height slider is dragged, anything positioned relative to the
    // camera needs to be updated.
    context.scene_context->height_slider->GetValueChanged().AddObserver(
        this, [&](Widget &, const float &val){
            MoveFakeCursorTo_(cursor_pos_);
            UpdateCaption_();
            UpdateHighlight_();
        });

    return true;
}

bool CaptureScriptApp::ProcessInstruction(const ScriptBase::Instr &instr) {
    if (instr.name == "caption") {
        const auto &cinst = GetTypedInstr_<CaptureScript::CaptionInstr>(instr);
        DisplayCaption_(cinst.text, cinst.pos, cinst.seconds);
    }
    else if (instr.name == "click") {
        GetEmitter().AddClick(cursor_pos_);
    }
    else if (instr.name == "cursor") {
        const auto &cinst = GetTypedInstr_<CaptureScript::CursorInstr>(instr);
        cursor_->SetEnabled(cinst.is_on);
    }
    else if (instr.name == "drag") {
        const auto &dinst = GetTypedInstr_<CaptureScript::DragInstr>(instr);
        DragTo_(dinst.motion, dinst.seconds, dinst.button);
    }
    else if (instr.name == "highlight") {
        const auto &hinst =
            GetTypedInstr_<CaptureScript::HighlightInstr>(instr);
        DisplayHighlight_(hinst.rect, hinst.seconds);
    }
    else if (instr.name == "highlightobj") {
        const auto &hinst =
            GetTypedInstr_<CaptureScript::HighlightObjInstr>(instr);
        Range2f rect;
        if (! GetNodeRect(hinst.object_name, hinst.margin, rect))
            return false;
        DisplayHighlight_(rect, hinst.seconds);
    }
    else if (instr.name == "key") {
        const auto &kinst = GetTypedInstr_<CaptureScript::KeyInstr>(instr);
        GetEmitter().AddKey(kinst.key_name, kinst.modifiers);
    }
    else if (instr.name == "mod") {
        const auto &minst = GetTypedInstr_<CaptureScript::ModInstr>(instr);
        GetEmitter().SetModifiedMode(minst.is_on);
    }
    else if (instr.name == "moveover") {
        const auto &minst = GetTypedInstr_<CaptureScript::MoveOverInstr>(instr);
        MoveOver_(minst.object_name, minst.seconds);
    }
    else if (instr.name == "moveto") {
        const auto &minst = GetTypedInstr_<CaptureScript::MoveToInstr>(instr);
        MoveTo_(minst.pos, minst.seconds);
    }
    else if (instr.name == "section") {
        const auto &sinst = GetTypedInstr_<CaptureScript::SectionInstr>(instr);
        if (video_writer_) {
            if (GetOptions_().report) {
                std::cout << "    Section " << video_writer_->GetChapterCount()
                          << " (" << sinst.tag << ") at "
                          << video_writer_->GetImageCount()
                          << ": " << sinst.title << "\n";
            }
            video_writer_->AddChapter(sinst.tag, sinst.title);
        }
    }
    else if (instr.name == "start") {
        is_capturing_ = true;
    }
    else if (instr.name == "tooltips") {
        const auto &tinst = GetTypedInstr_<CaptureScript::TooltipsInstr>(instr);
        TooltipFeedback::SetDelay(tinst.is_on ? 1 : 0);
    }
    else if (instr.name == "wait") {
        const auto &winst = GetTypedInstr_<CaptureScript::WaitInstr>(instr);
        MoveTo_(cursor_pos_, winst.seconds);
    }
    else {
        ASSERTM(false, "Unknown instruction type: " + instr.name);
        return false;
    }
    return true;
}

void CaptureScriptApp::InstructionsDone() {
    // Disable the handlers so the fake cursor does not move any more (and
    // there is no reason to pause).
    cursor_handler_->SetEnabled(false);
    if (pause_handler_)
        pause_handler_->SetEnabled(false);

    // Write the resulting video if requested.
    if (video_writer_)
        video_writer_->WriteToFile();

    // Remove the VideoWriter_ so this does not happen again in case
    // InstructionsDone() is called again (if remain flag is set).
    video_writer_.reset();

    instructions_done_ = true;
}

void CaptureScriptApp::BeginFrame() {
    // Update the caption and highlight rectangle if visible.
    if (! instructions_done_) {
        UpdateCaption_();
        UpdateHighlight_();
    }
}

void CaptureScriptApp::EndFrame() {
    if (video_writer_ && is_capturing_) {
        const auto image = GetRenderer().ReadImage(
            Range2i::BuildWithSize(Point2i(0, 0), GetWindowSize()));
        // Rows of image need to be inverted (GL vs stblib).
        ion::image::FlipImage(image);
        video_writer_->AddImage(*image);
    }
}

const CaptureScriptApp::Options & CaptureScriptApp::GetOptions_() const {
    const auto &opts = GetOptions();
    ASSERT(dynamic_cast<const Options *>(&opts));
    return static_cast<const Options &>(opts);
}

void CaptureScriptApp::DisplayCaption_(const Str &text, const Point2f &pos,
                                       float seconds) {
    caption_.text->SetText(text);

    // Get the number of lines of text and adjust the Y scale to match.
    const size_t line_count = 1 + std::ranges::count(text, '\n');
    caption_.text->SetUniformScale(line_count);

    // Scale the background to be slightly larger than the text.
    const float kTextBGFactorX = 1.1f;
    const float kTextBGFactorY = 2;
    const auto text_size = line_count * caption_.text->GetTextSize();
    caption_.bg->SetScale(Vector3f(kTextBGFactorX * text_size[0],
                                   kTextBGFactorY * text_size[1], 1));

    caption_.node->SetEnabled(true);
    caption_.pos                = pos;
    caption_fade_data_.duration = seconds;
    caption_fade_data_.elapsed  = 0;
    UpdateCaption_();
}

void CaptureScriptApp::DisplayHighlight_(const Range2f &rect, float seconds) {
    // The highlight is created as a frame composed of 4 rectangles. The top
    // and bottom rectangles run the full width and the left/right rectangles
    // fit between them.
    auto t = SG::FindNodeUnderNode(*highlight_, "Top");
    auto b = SG::FindNodeUnderNode(*highlight_, "Bottom");
    auto l = SG::FindNodeUnderNode(*highlight_, "Left");
    auto r = SG::FindNodeUnderNode(*highlight_, "Right");

    // The highlight rectangle is in the image plane, so choose a width for
    // each piece that works.
    const float kWidth = .01f;

    const auto min    = GetImagePlanePoint_(rect.GetMinPoint());
    const auto max    = GetImagePlanePoint_(rect.GetMaxPoint());
    const auto size   = max - min;
    const auto center = .5f * (min + max);
    const Vector3f y_off(0, .5f * size[1], 0);
    const Vector3f x_off(.5f * size[0], 0, 0);

    t->SetScale(Vector3f(size[0] + kWidth, kWidth, 1));
    b->SetScale(Vector3f(size[0] + kWidth, kWidth, 1));
    l->SetScale(Vector3f(kWidth, size[1] - kWidth, 1));
    r->SetScale(Vector3f(kWidth, size[1] - kWidth, 1));

    t->TranslateTo(center + y_off);
    b->TranslateTo(center - y_off);
    l->TranslateTo(center - x_off);
    r->TranslateTo(center + x_off);

    highlight_->SetEnabled(true);
    highlight_fade_data_.duration = seconds;
    highlight_fade_data_.elapsed  = 0;
    UpdateHighlight_();
}

void CaptureScriptApp::DragTo_(const Vector2f &motion, float seconds,
                               const Str &button) {
    ASSERT(button == "L" || button == "M" || button == "R");

    const Point2f end_pos = cursor_pos_ + motion;

    // Determine the number of events to create over the duration.
    const size_t frames = seconds * GetOptions_().fps;
    std::vector<Point2f> points;
    points.reserve(frames);
    for (auto i : std::views::iota(0U, frames)) {
        const float t = static_cast<float>(i + 1) / frames;
        points.push_back(BezierInterp(t, cursor_pos_, end_pos));
    }

    // Emit the points.
    auto &emitter = GetEmitter();
    emitter.SetDragButton(button == "M" ? Event::Button::kMouse2 :
                          button == "R" ? Event::Button::kMouse3 :
                          Event::Button::kMouse1);
    emitter.AddDragPoint("start", points[0]);
    for (size_t i = 1; i + 1 < frames; ++i)
        emitter.AddDragPoint("continue", points[i]);
    emitter.AddDragPoint("end", points[frames - 1]);
}

void CaptureScriptApp::MoveOver_(const Str &object_name, float seconds) {
    // Get the path from the scene root to target object.
    SG::NodePath path = GetNodePath(object_name);

    // Project the center of the object in world coordinates onto the Frustum
    // image plane to get the point to move to.
    const auto center = SG::CoordConv(path).ObjectToRoot(Point3f::Zero());
    MoveTo_(GetFrustum().ProjectToImageRect(center), seconds);
}

void CaptureScriptApp::MoveTo_(const Point2f &pos, float seconds) {
    if (seconds > 0) {
        // Determine the number of events to create over the duration.
        const size_t frames = seconds * GetOptions_().fps;
        for (auto i : std::views::iota(0U, frames)) {
            const float t = static_cast<float>(i + 1) / frames;
            GetEmitter().AddHoverPoint(BezierInterp(t, cursor_pos_, pos));
        }
    }
    else {
        // No duration or no delay - just one point.
        GetEmitter().AddHoverPoint(pos);
    }
}

void CaptureScriptApp::MoveFakeCursorTo_(const Point2f &pos) {
    cursor_->TranslateTo(GetImagePlanePoint_(pos));
    cursor_pos_ = pos;
}

void CaptureScriptApp::UpdateCaption_() {
    if (caption_fade_data_.elapsed <= caption_fade_data_.duration) {
        const float alpha = UpdateFade_(*caption_.node, caption_fade_data_);
        Color c = caption_.text->GetColor();
        c[3] = alpha;
        caption_.text->SetTextColor(c);
        // XXXX Change this to fade bg as well.

        // Set the position and face the camera. Do this every time in case the
        // view changes.
        caption_.node->TranslateTo(GetImagePlanePoint_(caption_.pos));
        caption_.node->SetRotation(GetFrustum().orientation);
    }
}

void CaptureScriptApp::UpdateHighlight_() {
    if (highlight_fade_data_.elapsed <= highlight_fade_data_.duration) {
        const float alpha = UpdateFade_(*highlight_, highlight_fade_data_);
        Color c = highlight_->GetBaseColor();
        c[3] = alpha;
        highlight_->SetBaseColor(c);
    }
}

float CaptureScriptApp::UpdateFade_(SG::Node &node, FadeData_ &fade_data) {
    // Fade-in/out time.
    const float kFadeTime = .5f;

    float alpha = 1;
    if (fade_data.elapsed < kFadeTime) {
        // Just starting, so fade in.
        alpha = fade_data.elapsed / kFadeTime;
    }
    else if (fade_data.elapsed >= fade_data.duration - kFadeTime) {
        // Almost done, so fade out.
        alpha = (fade_data.duration - fade_data.elapsed) / kFadeTime;
    }

    fade_data.elapsed += 1. / GetOptions_().fps;

    // Completely done?
    if (fade_data.elapsed >= fade_data.duration)
        node.SetEnabled(false);

    return alpha;
}

Point3f CaptureScriptApp::GetImagePlanePoint_(const Point2f &pos) {
    // Build a ray through the point using the view frustum.
    const Frustum frustum = GetFrustum();
    const auto ray = frustum.BuildRay(pos);

    // Intersect the ray with the plane 1 unit away along the view direction
    // and translate the fake cursor to the resulting point.
    const Vector3f view_dir = frustum.GetViewDirection();
    const Plane plane(frustum.position + view_dir, view_dir);

    float distance;
    RayPlaneIntersect(ray, plane, distance);
    return ray.GetPoint(distance);
}

Frustum CaptureScriptApp::GetFrustum() const {
    // Get the WindowCamera from the Scene and let it build a Frustum.
    const auto &sc = *GetContext().scene_context;
    auto cam = sc.scene->GetTypedCamera<SG::WindowCamera>();
    ASSERT(cam);
    Frustum frustum;
    cam->BuildFrustum(GetWindowSize(), frustum);
    return frustum;
}
