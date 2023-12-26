#include <iostream>

#include <ion/gfxutils/printer.h>
#include <ion/gfxutils/shadermanager.h>

#include "App/Args.h"
#include "App/RegisterTypes.h"
#include "App/SceneLoader.h"
#include "Base/Event.h"
#include "Debug/Print.h"
#include "Debug/Shortcuts.h"
#include "Handlers/BoardHandler.h"
#include "Handlers/MainHandler.h"
#include "Handlers/ViewHandler.h"
#include "Managers/EventManager.h"
#include "Managers/SceneContext.h"
#include "Managers/SettingsManager.h"
#include "Math/Linear.h"
#include "Math/Types.h"
#include "Panels/Board.h"
#include "Panels/Panel.h"
#include "Place/ClickInfo.h"
#include "Place/PrecisionStore.h"
#include "SG/Intersector.h"
#include "SG/IonContext.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/WindowCamera.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/Tuning.h"
#include "Viewers/GLFWViewer.h"
#include "Viewers/Renderer.h"
#include "Widgets/ClickableWidget.h"

// These are needed for special case code to examine specific Panels.
#include "Panels/DialogPanel.h"
#include "Panels/FilePanel.h"
#include "Panels/RadialMenuPanel.h"
#include "Panels/SettingsPanel.h"

/// \file
/// The nodeviewer application can be used to interactively view EMD nodes for
/// testing purposes without having to run the main application. See the usage
/// string for details.
///
/// \ingroup Apps

// ----------------------------------------------------------------------------
// Exception handling.
// ----------------------------------------------------------------------------

static void HandleEx_(const Str &when, const std::exception &ex) {
#if RELEASE_BUILD
    std::cerr << "*** Caught exception:\n" << ex.what() << "\n";
#else
    if (const auto *aex = dynamic_cast<const AssertException *>(&ex)) {
        std::cerr << "*** Caught assertion exception " << when << ":\n"
                  << ex.what() << "\n";
        std::cerr << "*** STACK:\n";
        for (const auto &s: aex->GetStackTrace())
            std::cerr << "  " << s << "\n";
    }
    else {
        std::cerr << "*** Caught exception " << when << ":\n"
                  << ex.what() << "\n";
    }
#endif
}

// ----------------------------------------------------------------------------
// EventHandler_ class.
// ----------------------------------------------------------------------------

// An instance of this class is added first to the Handler list. It just
// invokes a function inside the Application_ class.
class AppHandler_ : public Handler {
  public:
    using HandlerFunc = std::function<bool(const Event &event)>;
    explicit AppHandler_(const HandlerFunc &func) : func_(func) {
        ASSERT(func);
    }
    virtual HandleCode HandleEvent(const Event &event) override {
        return func_(event) ?
            HandleCode::kHandledStop : HandleCode::kNotHandled;
    }
  private:
    HandlerFunc func_;
};

DECL_SHARED_PTR(AppHandler_);

// ----------------------------------------------------------------------------
// Application_ class.
// ----------------------------------------------------------------------------

class Application_ {
  public:
    explicit Application_(const Args &args);
    ~Application_();
    bool InitScene();
    bool InitViewer(const Vector2i &window_size);
    void MainLoop();

  private:
    const Args          &args_;
    bool                is_fixed_camera_ = false;
    EventManagerPtr     event_manager_;
    PrecisionStorePtr precision_store_;
    SceneLoader         loader_;
    SG::ScenePtr        scene_;
    SceneContextPtr     scene_context_;
    SG::Hit             hit_;
    SG::NodePtr         intersection_sphere_;
    SG::NodePath        path_to_node_;  /// Path to node to be viewed.
    SG::WindowCameraPtr camera_;
    GLFWViewerPtr       glfw_viewer_;
    RendererPtr         renderer_;
    AppHandler_Ptr      app_handler_;
    BoardHandlerPtr     board_handler_;
    MainHandlerPtr      main_handler_;
    ViewHandlerPtr      view_handler_;

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

Application_::~Application_() {
    event_manager_->ClearHandlers();

    // Instances must be destroyed in a particular order.
#if ENABLE_DEBUG_FEATURES
    Debug::ShutDown();
#endif
    view_handler_.reset();
    scene_context_.reset();
    renderer_.reset();
    glfw_viewer_.reset();
}

bool Application_::InitScene() {
    const FilePath scene_path = FilePath::GetResourcePath(
        "scenes", "nodeviewer" + TK::kDataFileExtension);
    scene_ = loader_.LoadScene(scene_path);
    if (! scene_)
        return false;
    scene_context_.reset(new SceneContext);

    // Set up managers.
    precision_store_.reset(new PrecisionStore);
    event_manager_.reset(new EventManager);

    // Set up event handlers. Order is important here.
    app_handler_.reset(new AppHandler_(
                           [&](const Event &ev){ return HandleEvent_(ev); }));
    view_handler_.reset(new ViewHandler);
    view_handler_->SetFixedCameraPosition(is_fixed_camera_);
    board_handler_.reset(new BoardHandler);
    main_handler_.reset(new MainHandler(false));
    main_handler_->SetPrecisionStore(precision_store_);
    main_handler_->GetClicked().AddObserver(
        this, [&](const ClickInfo &info){ ProcessClick_(info); });
    event_manager_->AppendHandler(app_handler_);
    event_manager_->AppendHandler(view_handler_);
    event_manager_->AppendHandler(board_handler_);
    event_manager_->AppendHandler(main_handler_);

    SetUpScene_();
    UpdateScene_();
    ResetView_();

    return true;
}

bool Application_::InitViewer(const Vector2i &window_size) {
    auto error_func = [](const Str &error){
        std::cerr << "*** " << error << "\n";
    };
    glfw_viewer_.reset(new GLFWViewer(error_func));
    if (! glfw_viewer_->Init(window_size, false)) {
        glfw_viewer_.reset();
        return false;
    }

    // Set up the renderer.
    renderer_.reset(new Renderer(loader_.GetShaderManager(), true));

    return true;
}

void Application_::MainLoop() {
    std::vector<Event> events;
    while (! should_quit_) {
        renderer_->BeginFrame();
        const bool is_modified_mode = glfw_viewer_->IsShiftKeyPressed();

        main_handler_->ProcessUpdate(is_modified_mode);

        events.clear();
        glfw_viewer_->SetPollEventsFlag(need_render_ ||
                                        ! main_handler_->IsWaiting());
        glfw_viewer_->EmitEvents(events);
        if (! event_manager_->HandleEvents(events, is_modified_mode,
                                           TK::kMaxEventHandlingTime))
            should_quit_ = true;

        // Render
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

    // Track mouse intersection with intersection_sphere_.
    UpdateIntersectionSphere_(event);

    // Handle key presses.
    bool handled = false;
    if (event.flags.Has(Event::Flag::kKeyPress)) {
        const Str key_string = event.GetKeyString();
#if ENABLE_DEBUG_FEATURES
        if (key_string == "Alt-r") {
            ReloadScene_();
            handled = true;
        }
        else if (Debug::HandleShortcut(key_string))
            handled = true;
#endif
        else if (key_string == "Ctrl-q") {
            should_quit_ = true;
            handled = true;
        }
        else if (key_string == "Ctrl-v") {
            view_handler_->ResetView();
            handled = true;
        }
        else if (key_string == "Ctrl-o") {
            IntersectCenterRay_();
            handled = true;
        }
    }
    return handled;
}

void Application_::ProcessClick_(const ClickInfo &info) {
    KLOG('k', "Click on widget "
         << info.widget << " is_alt = " << info.is_modified_mode
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
        HandleEx_("reloading scene", ex);
    }
}

void Application_::SetUpScene_() {
    ASSERT(scene_);
    ASSERT(scene_context_);
    scene_context_->FillFromScene(scene_, false);
    scene_context_->path_to_stage = SG::NodePath(scene_->GetRootNode());
    path_to_node_ = SG::FindNodePathInScene(*scene_, "NodeViewerRoot");
#if ENABLE_DEBUG_FEATURES
    Debug::SetSceneContext(scene_context_);
    Debug::SetLimitPath(path_to_node_);
#endif

    // Add node if requested.
    const Str name = args_.GetString("--add");
    if (! name.empty())
        path_to_node_.back()->AddChild(SG::FindNodeInScene(*scene_, name));

    // Set up board and panel if requested.
    const Str board_name = args_.GetString("--board");
    const Str panel_name = args_.GetString("--panel");
    if (! board_name.empty() && ! panel_name.empty()) {
        auto panel = SG::FindTypedNodeInScene<Panel>(*scene_, panel_name);

        // Always set a context.
        SettingsManagerPtr settings_manager(new SettingsManager);
        Panel::ContextPtr pc(new Panel::Context);
        pc->settings_agent = settings_manager;
        panel->SetTestContext(pc);

        // Special case for FilePanel: set up path to something real.
        if (auto file_panel = std::dynamic_pointer_cast<FilePanel>(panel)) {
            file_panel->SetInitialPath(FilePath::GetHomeDirPath());
            file_panel->SetFileFormats(std::vector<FileFormat>{
                    FileFormat::kTextSTL,
                    FileFormat::kBinarySTL,
                });
        }
        // Special case for DialogPanel.
        if (auto dialog_panel = std::dynamic_pointer_cast<DialogPanel>(panel)) {
            dialog_panel->SetMessage("This is a temporary message!");
            dialog_panel->SetChoiceResponse("No", "Yes", true);
        }
        auto board = SG::FindTypedNodeInScene<Board>(*scene_, board_name);
        board->SetPanel(panel);
        board->Show(true);

        // Set up the BoardHandler.
        board_handler_->ClearBoards();
        board_handler_->AddBoard(board);
    }

    // Set up the IntersectionSphere.
    InitIntersectionSphere_(SG::FindNodeInScene(*scene_, "IntersectionSphere"));

    // Set up the frustum in the SceneContext.
    ASSERT(glfw_viewer_);
    scene_context_->frustum = glfw_viewer_->GetFrustum();

    // Initialize the MainHandler::Context.
    ASSERT(main_handler_);
    MainHandler::Context mc;
    mc.scene            = scene_context_->scene;
    mc.frustum          = scene_context_->frustum;
    mc.path_to_stage    = scene_context_->path_to_stage;
    mc.left_controller  = scene_context_->left_controller;
    mc.right_controller = scene_context_->right_controller;
    mc.debug_sphere     = scene_context_->debug_sphere;
    main_handler_->SetContext(mc);

    // Now that everything has been found, disable searching through the
    // "Definitions" Node.
    SG::FindNodeInScene(*scene_, "Definitions")->SetFlagEnabled(
        SG::Node::Flag::kSearch, false);

    // Check for changes to the root node to trigger rendering.
    scene_->GetRootNode()->GetChanged().AddObserver(
        this, [&](SG::Change, const SG::Object &){ need_render_ = true; });

    renderer_->Reset(*scene_);
}

void Application_::UpdateScene_() {
    ASSERT(scene_);

    // Install the window camera in the viewer.
    auto gantry = scene_->GetGantry();
    ASSERT(gantry);
    for (auto &cam: gantry->GetCameras()) {
        if (cam->GetTypeName() == "WindowCamera") {
            camera_ = std::dynamic_pointer_cast<SG::WindowCamera>(cam);
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
    intersection_sphere_->SetEnabled(false);
}

void Application_::UpdateIntersectionSphere_(const Event &event) {
    if (event.device == Event::Device::kMouse &&
        event.flags.Has(Event::Flag::kPosition2D)) {
        const Ray ray = scene_context_->frustum->BuildRay(event.position2D);
        hit_ = SG::Intersector::IntersectScene(*scene_, ray);
        const bool got_hit = ! hit_.path.empty();
        intersection_sphere_->SetEnabled(got_hit);
        if (got_hit)
            intersection_sphere_->TranslateTo(hit_.GetWorldPoint());
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
    const Bounds wb = TransformBounds(
        lb, SG::CoordConv(path_to_node_).GetObjectToRootMatrix());
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
    const Bounds wb = TransformBounds(
        lb, SG::CoordConv(path_to_node_).GetObjectToRootMatrix());

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
        std::cout << "Ray " << ray << " intersected nothing\n";
    }
    else {
        std::cout << "Ray " << ray << " intersected "
                  << hit.path.ToString() << "\n";
    }
}

// ----------------------------------------------------------------------------
// Mainline.
// ----------------------------------------------------------------------------

constinit const char kUsageString[] =
R"(nodeviewer: a test program for viewing nodes

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
    Util::app_type = Util::AppType::kInteractive;

    Args args(argc, argv, kUsageString);

    KLogger::SetKeyString(args.GetString("--klog"));

    {
        Application_ app(args);
        try {
            if (! app.InitViewer(Vector2i(800, 600)) || ! app.InitScene())
                return 1;
            app.MainLoop();
        }
        catch (std::exception &ex) {
            HandleEx_("in app", ex);
        }
    }
    return 0;
}
