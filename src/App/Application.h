#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"

DECL_SHARED_PTR(SessionManager);

class Controller;
class LogHandler;
struct TestContext;

/// The Application class manages the entire application.
///
/// \ingroup App
class Application {
  public:
    Application();
    ~Application();

    /// Initializes the application.
    bool Init(const Vector2i &window_size, bool do_ion_remote);

    /// Enters the main loop.
    void MainLoop();

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

  private:
    class  Loader_;
    class  Impl_;
    std::unique_ptr<Impl_> impl_;
};
