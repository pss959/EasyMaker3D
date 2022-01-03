#include <iostream>

#include <docopt/docopt.h>

#include <ion/gfxutils/printer.h>
#include <ion/gfxutils/shadermanager.h>

#include "Debug/Print.h"
#include "Event.h"
#include "Handlers/MainHandler.h"
#include "Handlers/ViewHandler.h"
#include "IO/Reader.h"
#include "Managers/ColorManager.h"
#include "Managers/PrecisionManager.h"
#include "Math/Linear.h"
#include "Math/Types.h"
#include "Procedural.h"
#include "RegisterTypes.h"
#include "Renderer.h"
#include "SG/IonContext.h"
#include "SG/Node.h"
#include "SG/ProceduralImage.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/Tracker.h"
#include "SG/WindowCamera.h"
#include "SceneContext.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/KLog.h"
#include "Viewers/GLFWViewer.h"

// ----------------------------------------------------------------------------
// Loader_ class.
// ----------------------------------------------------------------------------

class Loader_ {
  public:
    Loader_();
    SG::ScenePtr LoadScene(const Util::FilePath &path);
    SG::NodePtr  LoadNode(const Util::FilePath &path);

    const ion::gfxutils::ShaderManagerPtr & GetShaderManager() {
        return shader_manager_;
    }

  private:
    SG::TrackerPtr                  tracker_;
    ion::gfxutils::ShaderManagerPtr shader_manager_;
    ion::text::FontManagerPtr       font_manager_;
    SG::IonContextPtr               ion_context_;
};

Loader_::Loader_() : tracker_(new SG::Tracker),
    shader_manager_(new ion::gfxutils::ShaderManager),
    font_manager_(new ion::text::FontManager),
    ion_context_(new SG::IonContext) {

    ion_context_->SetTracker(tracker_);
    ion_context_->SetShaderManager(shader_manager_);
    ion_context_->SetFontManager(font_manager_);
}

SG::ScenePtr Loader_::LoadScene(const Util::FilePath &path) {
    // Wipe out all previous shaders to avoid conflicts.
    shader_manager_.Reset(new ion::gfxutils::ShaderManager);
    ion_context_->Reset();
    ion_context_->SetShaderManager(shader_manager_);

    SG::ScenePtr scene;
    try {
        Reader reader;
        scene = reader.ReadScene(path, *tracker_);
        scene->SetUpIon(ion_context_);
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception loading scene:\n"
                  << ex.what() << "\n";
        scene.reset();
    }
    return scene;
}

// ----------------------------------------------------------------------------
// Application_ class.
// ----------------------------------------------------------------------------

class Application_ {
  public:
    bool Init(const Vector2i &window_size, const std::string &node_to_add);
    void MainLoop();

  private:
    PrecisionManagerPtr precision_manager_;
    Loader_             loader_;
    SG::ScenePtr        scene_;
    SceneContextPtr     scene_context_;
    SG::NodePath        path_to_node_;  /// Path to node to be viewed.
    SG::WindowCameraPtr camera_;
    GLFWViewerPtr       glfw_viewer_;
    RendererPtr         renderer_;
    MainHandlerPtr      main_handler_;
    ViewHandlerPtr      view_handler_;

    bool should_quit_ = false;

    bool HandleEvent_(const Event &event);
    void ReloadScene_();
    void UpdateScene_();
    void ResetView_();
    void PrintBounds_();
    void PrintCamera_();
    void PrintIonGraph_();
};

bool Application_::Init(const Vector2i &window_size,
                        const std::string &node_to_add) {
    SG::ProceduralImage::AddFunction(
        "GenerateGridImage", std::bind(GenerateGridImage, 32));
    RegisterTypes();

    const Util::FilePath scene_path =
        Util::FilePath::GetResourcePath("scenes", "nodeviewer.mvn");
    scene_ = loader_.LoadScene(scene_path);
    if (! scene_)
        return false;
    scene_context_.reset(new SceneContext);
    scene_context_->FillFromScene(scene_, false);
    path_to_node_ = SG::FindNodePathInScene(*scene_, "NodeViewerRoot");
    ASSERT(! path_to_node_.empty());

    if (! node_to_add.empty())
        path_to_node_.back()->AddChild(
            SG::FindNodeInScene(*scene_, node_to_add));

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

    view_handler_.reset(new ViewHandler);
    view_handler_->SetFixedCameraPosition(false);

    UpdateScene_();
    ResetView_();

    return true;
}

void Application_::MainLoop() {
    std::vector<Event> events;
    while (! should_quit_) {
        const bool is_alternate_mode = glfw_viewer_->IsShiftKeyPressed();

        // Update the frustum used for intersection testing.
        scene_context_->frustum = glfw_viewer_->GetFrustum();

        main_handler_->ProcessUpdate(is_alternate_mode);

        events.clear();
        glfw_viewer_->EmitEvents(events);
        for (auto &event: events) {
            if (event.flags.Has(Event::Flag::kExit)) {
                should_quit_ = true;
                break;
            }
            HandleEvent_(event) ||
                view_handler_->HandleEvent(event) ||
                main_handler_->HandleEvent(event);
        }

        // Render to all viewers.
        glfw_viewer_->Render(*scene_, *renderer_);
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
        if (key_string == "<Ctrl>b") {
            PrintBounds_();
            return true;
        }
        else if (key_string == "<Ctrl>c") {
            PrintCamera_();
            return true;
        }
        else if (key_string == "<Ctrl>i") {
            PrintIonGraph_();
            return true;
        }
        else if (key_string == "<Ctrl>n") {
            Debug::PrintNodesAndShapes(*scene_->GetRootNode(), false);
            return true;
        }
        else if (key_string == "<Ctrl>q") {
            should_quit_ = true;
            return true;
        }
        else if (key_string == "<Ctrl>r") {
            ReloadScene_();
            return true;
        }
        else if (key_string == "<Ctrl>v") {
            view_handler_->ResetView();
            return true;
        }
    }

    return false;
}

void Application_::ReloadScene_() {
    ASSERT(scene_);
    ColorManager::ClearSpecialColors();
    view_handler_->Reset();
    glfw_viewer_->FlushPendingEvents();
    try {
        scene_ = loader_.LoadScene(scene_->GetPath());
        scene_context_->FillFromScene(scene_, false);
        path_to_node_ = SG::FindNodePathInScene(*scene_, "NodeViewerRoot");
        renderer_->Reset(*scene_);
        UpdateScene_();
        ResetView_();
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception reloading scene:\n"
                  << ex.what() << "\n";
    }
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
    const Bounds wb = TransformBounds(lb, path_to_node_.GetFromLocalMatrix());
    const float radius = kRadiusScale *
        .5f * ion::math::Length(wb.GetMaxPoint() - wb.GetMinPoint());

    const float near_value = kNearFrac * radius;
    camera_->SetPosition(wb.GetCenter() + Vector3f(0, 0, radius + near_value));
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

void Application_::PrintBounds_() {
    const Bounds lb = path_to_node_.back()->GetBounds();
    const Bounds wb = TransformBounds(lb, path_to_node_.GetFromLocalMatrix());

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

// ----------------------------------------------------------------------------
// Mainline.
// ----------------------------------------------------------------------------

static const char USAGE[] =
R"(nodeviewer: a test program for viewing IMakerVR nodes

    Usage:
      nodeviewer [<node_to_add>]

    If node_to_add is supplied, it is the name of a node to add under
    NodeViewerRoot. Otherwise, the contents of NodeViewerRoot.mvn are used as
    is.
)";

int main(int argc, const char** argv)
{
    std::map<std::string, docopt::value> args = docopt::docopt(
        USAGE,
        { argv + 1, argv + argc },
        true,                     // Show help if requested
        "IMakerVR Version XXXX");

    const auto &arg = args["<node_to_add>"];
    const std::string node_to_add = arg && arg.isString() ? arg.asString() : "";

    KLogger::SetKeyString("");  // Add characters to help debug.
    Application_ app;
    try {
        if (! app.Init(Vector2i(800, 600), node_to_add))
            return 1;
        app.MainLoop();
    }
    catch (AssertException &ex) {
        std::cerr << "*** Caught assertion exception:\n" << ex.what() << "\n";
    }
    return 0;
}
