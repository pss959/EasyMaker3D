#include <iostream>

#include <ion/gfxutils/printer.h>
#include <ion/gfxutils/shadermanager.h>

#include "App/Args.h"
#include "App/ClickInfo.h"
#include "App/RegisterTypes.h"
#include "App/Renderer.h"
#include "App/SceneContext.h"
#include "App/SceneLoader.h"
#include "Base/Event.h"
#include "Debug/Print.h"
#include "Handlers/BoardHandler.h"
#include "Handlers/MainHandler.h"
#include "Handlers/ViewHandler.h"
#include "Items/Board.h"
#include "Managers/PrecisionManager.h"
#include "Managers/SettingsManager.h"
#include "Math/Linear.h"
#include "Math/Types.h"
#include "Panels/Panel.h"
#include "SG/Intersector.h"
#include "SG/IonContext.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/Tracker.h"
#include "SG/WindowCamera.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/KLog.h"
#include "Util/VersionInfo.h"
#include "Viewers/GLFWViewer.h"
#include "Widgets/ClickableWidget.h"

// These are needed for special case code to examine specific Panels.
#include "Panels/DialogPanel.h"
#include "Panels/FilePanel.h"
#include "Panels/RadialMenuPanel.h"
#include "Panels/SettingsPanel.h"

// ----------------------------------------------------------------------------
// Application_ class.
// ----------------------------------------------------------------------------

class Application_ {
  public:
    explicit Application_(const Args &args);
    bool InitScene();
    bool InitViewer(const Vector2i &window_size);
    void MainLoop();

  private:
    const Args          &args_;
    bool                is_fixed_camera_ = false;
    PrecisionManagerPtr precision_manager_;
    SceneLoader         loader_;
    SG::ScenePtr        scene_;
    SceneContextPtr     scene_context_;
    SG::Hit             hit_;
    SG::NodePtr         intersection_sphere_;
    SG::NodePath        path_to_node_;  /// Path to node to be viewed.
    SG::WindowCameraPtr camera_;
    GLFWViewerPtr       glfw_viewer_;
    RendererPtr         renderer_;
    MainHandlerPtr      main_handler_;
    ViewHandlerPtr      view_handler_;

    /// All Handlers, in order.
    std::vector<HandlerPtr> handlers_;

    bool need_render_ = true;
    bool should_quit_ = false;

    bool HandleEvent_(const Event &event);
    void ProcessClick_(const ClickInfo &info);
    void ReloadScene_();
    void SetUpScene_();
    void UpdateScene_();
    void InitIntersectionSphere_(const SG::NodePtr &node);
    void UpdateIntersectionSphere_(const Event &event);
    void ResetView_();
    void PrintBounds_();
    void PrintCamera_();
    void PrintIntersection_();
    void PrintIonGraph_();
    void IntersectCenterRay_();
};

Application_::Application_(const Args &args) : args_(args) {
    RegisterTypes();

    is_fixed_camera_ = args_.GetBool("--fixed_camera");
}

bool Application_::InitScene() {
    const FilePath scene_path =
        FilePath::GetResourcePath("scenes", "nodeviewer.mvn");
    scene_ = loader_.LoadScene(scene_path);
    if (! scene_)
        return false;
    scene_context_.reset(new SceneContext);

    SetUpScene_();

    return true;
}

bool Application_::InitViewer(const Vector2i &window_size) {
    glfw_viewer_.reset(new GLFWViewer);
    if (! glfw_viewer_->Init(window_size)) {
        glfw_viewer_.reset();
        return false;
    }

    precision_manager_.reset(new PrecisionManager);

    // Set up the renderer.
    renderer_.reset(new Renderer(loader_.GetShaderManager(), true));
    renderer_->Reset(*scene_);

    main_handler_.reset(new MainHandler);
    main_handler_->SetPrecisionManager(precision_manager_);
    main_handler_->SetSceneContext(scene_context_);
    main_handler_->GetClicked().AddObserver(
        this, [&](const ClickInfo &info){ ProcessClick_(info); });

    view_handler_.reset(new ViewHandler);
    view_handler_->SetFixedCameraPosition(is_fixed_camera_);

    handlers_.push_back(view_handler_);
    handlers_.push_back(main_handler_);

    UpdateScene_();
    ResetView_();

    return true;
}

void Application_::MainLoop() {
    std::vector<Event> events;
    while (! should_quit_) {
        renderer_->BeginFrame();
        const bool is_alternate_mode = glfw_viewer_->IsShiftKeyPressed();

        // Update the frustum used for intersection testing.
        scene_context_->frustum = glfw_viewer_->GetFrustum();

        main_handler_->ProcessUpdate(is_alternate_mode);

        events.clear();
        glfw_viewer_->SetPollEventsFlag(need_render_ ||
                                        ! main_handler_->IsWaiting());
        glfw_viewer_->EmitEvents(events);
        for (auto &event: events) {
            if (event.flags.Has(Event::Flag::kExit)) {
                should_quit_ = true;
                break;
            }
            UpdateIntersectionSphere_(event);

            if (! HandleEvent_(event)) {
                for (auto &handler: handlers_)
                    if (handler->HandleEvent(event))
                        break;
            }
        }

        // Render to all viewers.
        need_render_ = false;
        glfw_viewer_->Render(*scene_, *renderer_);
        renderer_->EndFrame();
    }
}

bool Application_::HandleEvent_(const Event &event) {
    // Scroll wheel zooms in and out.
    if (event.flags.Has(Event::Flag::kPosition1D) &&
        event.device == Event::Device::kMouse) {
        const float kDeltaFOV = 2;
        const float kMinFOV   = 40;
        const float kMaxFOV   = 120;
        float fov = camera_->GetFOV().Degrees();
        fov -= event.position1D * kDeltaFOV;
        fov = Clamp(fov, kMinFOV, kMaxFOV);
        camera_->SetFOV(Anglef::FromDegrees(fov));
        return true;
    }

    // Handle key presses.
    if (event.flags.Has(Event::Flag::kKeyPress)) {
        const std::string key_string = event.GetKeyString();
#if DEBUG
        if (Debug::ProcessPrintShortcut(key_string))
            return true;
        if (key_string == "<Alt>!")  // From ShortcutHandler.
            KLogger::ToggleLogging();
#endif
        if (key_string == "<Ctrl>b") {
            PrintBounds_();
            return true;
        }
        else if (key_string == "<Ctrl>c") {
            PrintCamera_();
            return true;
        }
        else if (key_string == "<Ctrl>i") {
            PrintIntersection_();
            return true;
        }
        else if (key_string == "<Ctrl>I") {
            PrintIonGraph_();
            return true;
        }
#if DEBUG
        else if (key_string == "<Ctrl>p") {
            Debug::PrintNodeGraph(*path_to_node_.back(), false);
            return true;
        }
#endif
        else if (key_string == "<Ctrl>q") {
            should_quit_ = true;
            return true;
        }
        else if (key_string == "<Ctrl>r") {
            ReloadScene_();
            return true;
        }
        else if (key_string == "<Ctrl>s") {
            intersection_sphere_->SetFlagEnabled(
                SG::Node::Flag::kRender,
                ! intersection_sphere_->IsFlagEnabled(SG::Node::Flag::kRender));
            return true;
        }
        else if (key_string == "<Ctrl>v") {
            view_handler_->ResetView();
            return true;
        }
        else if (key_string == "<Ctrl>o") {
            IntersectCenterRay_();
            return true;
        }
    }

    return false;
}

void Application_::ProcessClick_(const ClickInfo &info) {
    KLOG('k', "Click on widget "
         << info.widget << " is_alt = " << info.is_alternate_mode
         << " is_long = " << info.is_long_press);
    if (info.widget && info.widget->IsInteractionEnabled())
        info.widget->Click(info);
}

void Application_::ReloadScene_() {
    ASSERT(scene_);
    view_handler_->Reset();
    glfw_viewer_->FlushPendingEvents();
    try {
        scene_ = loader_.LoadScene(scene_->GetPath());
        SetUpScene_();
        renderer_->Reset(*scene_);
        UpdateScene_();
        ResetView_();
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception reloading scene:\n"
                  << ex.what() << "\n";
    }
}

void Application_::SetUpScene_() {
    ASSERT(scene_);
    ASSERT(scene_context_);
    scene_context_->FillFromScene(scene_, false);
    scene_context_->path_to_stage = SG::NodePath(scene_->GetRootNode());
    path_to_node_ = SG::FindNodePathInScene(*scene_, "NodeViewerRoot");
#if DEBUG
    Debug::SetSceneContext(scene_context_);
    Debug::SetLimitPath(path_to_node_);
#endif

    // Add node if requested.
    const std::string name = args_.GetString("--add");
    if (! name.empty())
        path_to_node_.back()->AddChild(SG::FindNodeInScene(*scene_, name));

    // Set up board and panel if requested.
    const std::string board_name = args_.GetString("--board");
    const std::string panel_name = args_.GetString("--panel");
    if (! board_name.empty() && ! panel_name.empty()) {
        auto panel = SG::FindTypedNodeInScene<Panel>(*scene_, panel_name);

        // Always set a context.
        SettingsManagerPtr settings_manager(new SettingsManager);
        Panel::ContextPtr pc(new Panel::Context);
        pc->settings_manager = settings_manager;
        panel->SetTestContext(pc);

        // Special case for FilePanel: set up path to something real.
        if (auto file_panel = Util::CastToDerived<FilePanel>(panel)) {
            file_panel->SetInitialPath(FilePath::GetHomeDirPath());
            file_panel->SetFileFormats(std::vector<FileFormat>{
                    FileFormat::kTextSTL,
                    FileFormat::kBinarySTL,
                });
        }
        // Special case for DialogPanel.
        if (auto dialog_panel = Util::CastToDerived<DialogPanel>(panel)) {
            dialog_panel->SetMessage("This is a temporary message!");
            dialog_panel->SetChoiceResponse("No", "Yes");
        }
        auto board = SG::FindTypedNodeInScene<Board>(*scene_, board_name);
        board->SetPanel(panel);
        board->Show(true);

        // Add a BoardHandler to process events to test Pane input. It has to
        // come first.
        BoardHandlerPtr board_handler(new BoardHandler);
        board_handler->AddBoard(board);
        handlers_.insert(handlers_.begin(), board_handler);
    }

    // Set up the IntersectionSphere.
    InitIntersectionSphere_(SG::FindNodeInScene(*scene_, "IntersectionSphere"));

    // Now that everything has been found, disable searching through the
    // "Definitions" Node.
    SG::FindNodeInScene(*scene_, "Definitions")->SetFlagEnabled(
        SG::Node::Flag::kSearch, false);

    // Check for changes to the root node to trigger rendering.
    scene_->GetRootNode()->GetChanged().AddObserver(
        this, [&](SG::Change, const SG::Object &){ need_render_ = true; });
}

void Application_::UpdateScene_() {
    ASSERT(scene_);

    // Install the window camera in the viewer.
    auto gantry = scene_->GetGantry();
    ASSERT(gantry);
    for (auto &cam: gantry->GetCameras()) {
        if (cam->GetTypeName() == "WindowCamera") {
            camera_ = Util::CastToDerived<SG::WindowCamera>(cam);
            break;
        }
    }
    ASSERT(camera_);
    view_handler_->SetCamera(camera_);
    glfw_viewer_->SetCamera(camera_);
}

void Application_::InitIntersectionSphere_(const SG::NodePtr &node) {
    // Visible red color.
    node->SetBaseColor(Color(1, .4f, .4f));

    // Scale based on the viewed content size.
    const Bounds bounds = path_to_node_.back()->GetScaledBounds();
    const Vector3f size = bounds.GetSize();
    const float max_size = size[GetMaxElementIndex(size)];
    node->SetUniformScale(.04f * max_size);

    // Turn off until needed.
    node->SetEnabled(false);

    // Save for later use and start invisible.
    intersection_sphere_ = node;
    intersection_sphere_->SetFlagEnabled(SG::Node::Flag::kRender, false);
}

void Application_::UpdateIntersectionSphere_(const Event &event) {
    if (event.device == Event::Device::kMouse &&
        event.flags.Has(Event::Flag::kPosition2D)) {
        const Ray ray = scene_context_->frustum.BuildRay(event.position2D);
        hit_ = SG::Intersector::IntersectScene(*scene_, ray);
        const bool got_hit = ! hit_.path.empty();
        intersection_sphere_->SetEnabled(got_hit);
        if (got_hit)
            intersection_sphere_->SetTranslation(hit_.GetWorldPoint());
    }
}

void Application_::ResetView_() {
    ASSERT(! path_to_node_.empty());
    ASSERT(camera_);

    // CAM         NEAR                    CENTER                    FAR
    //   |           |                       |                        |
    //    ----------- ----------------------- ------------------------
    //       kNear          radius                    radius

    // Extra padding for radius to avoid near/far clipping.
    const float kRadiusScale = 1.1f;

    // Distance from camera to near plane as a fraction of radius.
    const float kNearFrac = .8f;

    // Change the view to encompass the object, looking along the -Z axis
    // toward the center.
    const Bounds lb = path_to_node_.back()->GetBounds();
    const Bounds wb =
        TransformBounds(lb, CoordConv(path_to_node_).GetObjectToRootMatrix());
    const float radius = kRadiusScale *
        .5f * ion::math::Length(wb.GetMaxPoint() - wb.GetMinPoint());

    const float near_value = kNearFrac * radius;
    if (! is_fixed_camera_)
        camera_->SetPosition(wb.GetCenter() +
                             Vector3f(0, 0, radius + near_value));
    camera_->SetOrientation(Rotationf::Identity());
    camera_->SetNearAndFar(near_value, near_value + 2 * radius);

    view_handler_->SetRotationCenter(wb.GetCenter());
}

void Application_::PrintCamera_() {
    ASSERT(camera_);
    ASSERT(view_handler_);
    const float radius = ion::math::Length(
        view_handler_->GetRotationCenter() - camera_->GetPosition());

    std::cout << "== Current camera:"
              << "\n   Position  = " << camera_->GetPosition()
              << "\n   Direction = " << camera_->GetViewDirection()
              << "\n   FOV       = " << camera_->GetFOV().Degrees()
              << "\n   Near      = " << camera_->GetNear()
              << "\n   Far       = " << camera_->GetFar()
              << "\n   Center    = " << view_handler_->GetRotationCenter()
              << "\n   Radius    = " << radius
              << "\n";
}

void Application_::PrintIntersection_() {
    if (hit_.path.empty())
        std::cerr << "No Intersection\n";
    else
        std::cerr << "Hit on " << hit_.path.ToString() << "\n";
}

void Application_::PrintBounds_() {
    const Bounds lb = path_to_node_.back()->GetBounds();
    const Bounds wb =
        TransformBounds(lb, CoordConv(path_to_node_).GetObjectToRootMatrix());

    std::cout << "== Path: " << path_to_node_.ToString()
              << "\n   Local Bounds: " << lb
              << "\n   World Bounds: " << wb << "\n";
}

void Application_::PrintIonGraph_() {
    std::cout << "== Viewed Node Ion Graph:\n";
    ion::gfxutils::Printer printer;
    printer.EnableAddressPrinting(false);
    printer.EnableFullShapePrinting(true);
    printer.SetFloatCleanTolerance(1e-5f);  // Clean values close to zero.
    printer.PrintScene(path_to_node_.back()->GetIonNode(), std::cout);
}

void Application_::IntersectCenterRay_() {
    // Shoot a ray through the center of the window and see what it hits.
    Ray ray(camera_->GetPosition(), camera_->GetViewDirection());
    const SG::Hit hit = SG::Intersector::IntersectScene(*scene_, ray);
    if (hit.path.empty()) {
        std::cout << "Ray " << ray.ToString() << " intersected nothing\n";
    }
    else {
        std::cout << "Ray " << ray.ToString()
                  << " intersected " << hit.path.ToString() << "\n";
    }
}

// ----------------------------------------------------------------------------
// Mainline.
// ----------------------------------------------------------------------------

static const char kUsageString[] =
R"(nodeviewer: a test program for viewing IMakerVR nodes

    Usage:
      nodeviewer [--klog=<klog_string>] [--add=<node_name>] [--fixed_camera]
                 [--board=<board_name> --panel=<panel_name>]

    Options:
      --klog=<string>      String is passed to KLogger::SetKeyString().
      --add=<node_name>    The named Node is added to NodeViewerRoot.
      --fixed_camera       Use a fixed-position camera.
      --board=<board_name> The named Board is found and enabled.
      --panel=<panel_name> The named Panel is added to the Board.
)";

int main(int argc, const char** argv)
{
    Args args(argc, argv, kUsageString);

    KLogger::SetKeyString(args.GetString("--klog"));

    Application_ app(args);
    try {
        if (! app.InitScene() || ! app.InitViewer(Vector2i(800, 600)))
            return 1;
        app.MainLoop();
    }
    catch (AssertException &ex) {
        std::cerr << "*** Caught assertion exception:\n" << ex.what() << "\n";
    }
    return 0;
}
