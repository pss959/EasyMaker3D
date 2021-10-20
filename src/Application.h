#pragma once

#include <memory>

#include "Enums/Hand.h"
#include "Math/Types.h"

class Controller;
class LogHandler;

/// The Application class manages the entire application.
class Application {
  public:
    Application();
    ~Application();

    /// Initializes the application.
    bool Init(const Vector2i &window_size);

    /// Enters the main loop.
    void MainLoop();

    /// Reloads the scene.
    void ReloadScene();

    /// Returns the LogHandler so it can be enabled or disabled.
    LogHandler & GetLogHandler() const;

    /// Returns true if virtual reality is active, meaning that a headset is
    /// connected.
    bool IsVREnabled() const;

    /// Temporary workaround for OpenXR xrDestroyInstance() problem.
    /// TODO: Remove this if OpenXR gets fixed.
    bool ShouldKillApp() const { return IsVREnabled(); }

  private:
    class  Loader_;
    class  Impl_;
    std::unique_ptr<Impl_> impl_;
};
