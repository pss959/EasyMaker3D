#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"

DECL_SHARED_PTR(SessionManager);

class Controller;
class LogHandler;
class Renderer;
struct TestContext;

/// The Application class manages the entire application.
///
/// \ingroup App
class Application {
  public:
    /// Application options.
    struct Options {
        Vector2i window_size;             /// Size of the application window.
        bool     do_ion_remote      = false;
        bool     fullscreen         = false;
        bool     ignore_vr          = false;
        bool     set_up_touch       = false;
        bool     show_session_panel = true;
    };

    Application();
    ~Application();

    /// Initializes the application.
    bool Init(const Options &options);

    /// Enters the main loop.
    void MainLoop();

    /// Processes a single frame during the main loop. Returns false if the
    /// main loop should exit.
    virtual bool ProcessFrame(size_t render_count);

    /// Reloads the scene.
    void ReloadScene();

    /// Returns the LogHandler so it can be enabled or disabled.
    LogHandler & GetLogHandler() const;

    /// Returns true if virtual reality is active, meaning that a headset is
    /// connected.
    bool IsVREnabled() const;

    /// Shuts down (especially VR which does not like to be crashed out of.
    void Shutdown();

  protected:
    /// Sets a flag indicating the Application will be used only for unit
    /// testing.
    void SetTestingFlag();

    /// Fills in a TestContext.
    void GetTestContext(TestContext &tc);

    /// Returns the Renderer.
    Renderer & GetRenderer();

  private:
    class  Loader_;
    class  Impl_;
    std::unique_ptr<Impl_> impl_;
};
