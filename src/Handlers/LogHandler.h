#pragma once

#include <vector>

#include "Base/Event.h"
#include "Handlers/Handler.h"
#include "Util/Enum.h"
#include "Util/Memory.h"

struct Log;

DECL_SHARED_PTR(LogHandler);

/// LogHandler is a derived Handler that logs events for debugging and
/// testing. It is disabled by default.
///
/// \ingroup Handlers
class LogHandler : public Handler {
  public:
    LogHandler();
    virtual ~LogHandler();

    /// Filters events by the given set of devices. If this set is empty, there
    /// is no filtering. Otherwise, only events with any of the specified
    /// devices will be logged.
    void SetDevices(const std::vector<Event::Device> &devices) {
        devices_ = devices;
    }

    /// Filters events by the given set of Event flags. If this set is 0, there
    /// is no filtering. Otherwise, only events with any of the specified flags
    /// will be logged.
    void SetFlags(const Util::Flags<Event::Flag> &flags) {
        flags_ = flags;
    }

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    /// Device filters.
    std::vector<Event::Device> devices_;

    /// Event Flag filters.
    Util::Flags<Event::Flag> flags_;

    /// Returns true if the given event passes the current filters, if any.
    bool PassesFilters_(const Event &event) const;
};
