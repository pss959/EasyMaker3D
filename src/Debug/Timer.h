#pragma once

#include <vector>

#include "Util/UTime.h"

namespace Debug {

/// The Timer class implements a cumulative timer that can help diagnose
/// performance issues.
///
/// \ingroup Debug
class Timer {
  public:
    /// The Timer is given a name to output in Report().
    explicit Timer(const Str &name);

    /// Resets the timer to 0 and removes all current timepoints.
    void Reset();

    /// Adds a named timepoint.
    void AddTimePoint(const Str &name);

    /// Reports on all timepoints and the total time since Reset() to stdout.
    void Report();

    /// Returns the cumulative time (in seconds) from the last call to Reset()
    /// to now.
    double GetTotalTime();

  private:
    struct TimePoint_ {
        Str   name;
        UTime time;
        TimePoint_() {}
        TimePoint_(const Str &n, const UTime &t) : name(n), time(t) {}
    };
    const Str               name_;
    UTime                   reset_time_;
    std::vector<TimePoint_> timepoints_;
};

}  // namespace Debug
