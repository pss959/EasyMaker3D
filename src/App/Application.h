#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"

class Controller;
class FilePath;
class LogHandler;
class Renderer;
struct TestContext;
DECL_SHARED_PTR(IEmitter);

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
    virtual bool Init(const Options &options);

    /// Enters the main loop.
    void MainLoop();

    /// Processes a single frame during the main loop. Returns false if the
    /// main loop should exit. If the force_poll flag is true, this forces the
    /// event-checking code to poll instead of waiting for the next event.
    virtual bool ProcessFrame(size_t render_count, bool force_poll);

    /// Reloads the scene.
    void ReloadScene();

    /// Returns the LogHandler so it can be enabled or disabled.
    LogHandler & GetLogHandler() const;

    /// Returns true if virtual reality is active, meaning that a headset is
    /// connected.
    bool IsVREnabled() const;

    /// Tells the SessionManager to save the current session to the given file
    /// with the given crash message and stack trace.
    void SaveCrashSession(const FilePath &path, const std::string &message,
                          const std::vector<std::string> &stack);

    /// Shuts down (especially VR which does not like to be crashed out of.
    void Shutdown();

  protected:
    /// Sets a flag indicating the Application will be used only for unit
    /// testing.
    void SetTestingFlag();

    /// Fills in a TestContext.
    void GetTestContext(TestContext &tc);

    /// Adds an event IEmitter to the list of emitters.
    void AddEmitter(const IEmitterPtr &emitter);

    /// Returns the current size of the application window, taking the
    /// fullscreen option into account.
    Vector2i GetWindowSize() const;

    /// Returns the Renderer.
    Renderer & GetRenderer();

    /// Allows derived classes to force touch mode to be on.
    void ForceTouchMode(bool is_on);

  private:
    class  Loader_;
    class  Impl_;
    std::unique_ptr<Impl_> impl_;
};
