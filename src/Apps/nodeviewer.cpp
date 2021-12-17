#include <iostream>

#include <ion/gfxutils/shadermanager.h>

#include "Event.h"
#include "Handlers/ViewHandler.h"
#include "IO/Reader.h"
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
#include "Util/Assert.h"
#include "Util/FilePath.h"
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
// KeyHandler_ class.
// ----------------------------------------------------------------------------

class KeyHandler_ : public Handler {
  public:
    void SetViewHandler(const ViewHandlerPtr &vh) { view_handler_ = vh; }
    bool ShouldQuit() const { return should_quit_; }
    virtual bool HandleEvent(const Event &event) override;
  private:
    ViewHandlerPtr view_handler_;
    bool should_quit_ = false;
};
typedef std::shared_ptr<KeyHandler_> KeyHandlerPtr_;

bool KeyHandler_::HandleEvent(const Event &event) {
    if (event.flags.Has(Event::Flag::kKeyPress)) {
        const std::string key_string = event.GetKeyString();
        if (key_string == "<Ctrl>q") {
            should_quit_ = true;
            return true;
        }
        else if (key_string == "<Ctrl>r") {
            // XXXX Reload
            return true;
        }
        else if (key_string == "<Ctrl>v") {
            view_handler_->ResetView();
            return true;
        }
    }

    return false;
}

// ----------------------------------------------------------------------------
// Application_ class.
// ----------------------------------------------------------------------------

class Application_ {
  public:
    bool Init(const Vector2i &window_size);
    void MainLoop();
    void ReloadScene();

  private:
    Loader_             loader_;
    SG::ScenePtr        scene_;
    SG::WindowCameraPtr camera_;
    GLFWViewerPtr       glfw_viewer_;
    RendererPtr         renderer_;
    ViewHandlerPtr      view_handler_;
    KeyHandlerPtr_      key_handler_;

    void UpdateScene_();
    void ResetView_();
};

bool Application_::Init(const Vector2i &window_size) {
    SG::ProceduralImage::AddFunction(
        "GenerateGridImage", std::bind(GenerateGridImage, 32));
    RegisterTypes();

    const Util::FilePath scene_path =
        Util::FilePath::GetResourcePath("scenes", "nodeviewer.mvn");
    scene_ = loader_.LoadScene(scene_path);
    if (! scene_)
        return false;

    glfw_viewer_.reset(new GLFWViewer);
    if (! glfw_viewer_->Init(window_size)) {
        glfw_viewer_.reset();
        return false;
    }

    // Set up the renderer.
    renderer_.reset(new Renderer(loader_.GetShaderManager(), true));
    renderer_->Reset(*scene_);

    view_handler_.reset(new ViewHandler);
    key_handler_.reset(new KeyHandler_);
    key_handler_->SetViewHandler(view_handler_);

    UpdateScene_();
    ResetView_();

    return true;
}

void Application_::MainLoop() {
    bool keep_running = true;
    std::vector<Event> events;
    while (keep_running && ! key_handler_->ShouldQuit()) {
        events.clear();
        glfw_viewer_->EmitEvents(events);
        for (auto &event: events) {
            if (event.flags.Has(Event::Flag::kExit)) {
                keep_running = false;
                break;
            }
            key_handler_->HandleEvent(event) ||
                view_handler_->HandleEvent(event);
        }

        // Render to all viewers.
        glfw_viewer_->Render(*scene_, *renderer_);
    }
}

void Application_::ReloadScene() {
    // XXXX
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
    ASSERT(scene_);
    ASSERT(camera_);

    // Change the view to encompass the object, looking along the -Z axis
    // toward the center.
    auto root = SG::FindNodeInScene(*scene_, "NodeViewerRoot");
    const Bounds bounds = root->GetBounds();
    const float radius =
        ion::math::Length(bounds.GetMaxPoint() - bounds.GetMinPoint());

    camera_->SetPosition(bounds.GetCenter() + Vector3f(0, 0, radius + 11));
    camera_->SetOrientation(Rotationf::Identity());
    camera_->SetNearAndFar(10, 2 * radius + 1);

    view_handler_->SetRotationCenter(bounds.GetCenter());
}

// ----------------------------------------------------------------------------
// Mainline.
// ----------------------------------------------------------------------------

int main() {
    Application_ app;
    try {
        if (! app.Init(Vector2i(800, 600)))
            return 1;
        app.MainLoop();
    }
    catch (AssertException &ex) {
        std::cerr << "*** Caught assertion exception:\n" << ex.what() << "\n";
    }
    return 0;
}
