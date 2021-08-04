#pragma once

#include "Interfaces/IHandler.h"

struct Log;

//! LogHandler is a derived IHandler that logs events for debugging and
//! testing. It is disabled by default.
class LogHandler : public IHandler {
  public:
    LogHandler();
    virtual ~LogHandler();

    virtual const char * GetClassName() const override { return "LogHandler"; }

    //! Enables or disables the handler. XXXX Move to a base class.
    void Enable(bool enable) { is_enabled_ = enable; }

    //! Returns whether the handler is enabled.
    bool IsEnabled() { return is_enabled_; }

    // ------------------------------------------------------------------------
    // IHandler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    //! Whether the handler is enabled.
    bool is_enabled_ = false;
};
