#pragma once

class FilePath;

/// IApplication is an interface for applications.
///
/// \ingroup App
class IApplication {
  public:
    /// Processes a single frame during the main loop. Returns false if the
    /// main loop should exit. If the force_poll flag is true, this forces the
    /// event-checking code to poll instead of waiting for the next event.
    virtual bool ProcessFrame(size_t render_count, bool force_poll) = 0;

    /// Returns true if the application is in modified input mode.
    virtual bool IsInModifiedMode() const = 0;

    /// Returns true if animation is enabled for resetting the height slider,
    /// the stage, or anything else that may animate.
    virtual bool IsAnimationEnabled() const = 0;

    /// Saves the current session to the \p path with a crash message and stack
    /// trace.
    virtual void SaveCrashSession(const FilePath &path, const Str &message,
                                  const StrVec &stack) = 0;

    /// Shuts the application down cleanly.
    virtual void Shutdown() = 0;
};
