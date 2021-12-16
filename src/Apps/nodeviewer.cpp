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
#include "SG/Tracker.h"
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
// Application_ class.
// ----------------------------------------------------------------------------

class Application_ {
  public:
    bool Init(const Vector2i &window_size);
    void MainLoop();
    void ReloadScene();

  private:
    Loader_       loader_;
    SG::ScenePtr  scene_;
    GLFWViewerPtr glfw_viewer_;
    RendererPtr   renderer_;
    ViewHandlerPtr view_handler_;
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
    renderer_.reset(new Renderer(loader_.GetShaderManager(), false));

    view_handler_.reset(new ViewHandler);

    // XXXX view_handler_->SetCamera(scene_context_->window_camera);

    // XXXX
    return true;
}

void Application_::MainLoop() {
    std::cerr << "XXXX Entering MainLoop()\n";
    bool keep_running = true;
    std::vector<Event> events;
    while (keep_running) {
        events.clear();
        glfw_viewer_->EmitEvents(events);
        for (auto &event: events) {
            if (event.flags.Has(Event::Flag::kExit)) {
                keep_running = false;
                break;
            }
            view_handler_->HandleEvent(event);
        }

        // Render to all viewers.
        glfw_viewer_->Render(*scene_, *renderer_);
    }
}

void Application_::ReloadScene() {
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
