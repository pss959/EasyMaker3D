//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

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

/// The StopTimer class is also a cumulative timer that can be stopped and
/// started. This is also useful for diagnosing performance issues.
///
/// \ingroup Debug
class StopTimer {
  public:
    /// The StopTimer is given a name to output in Report(). It is stopped by
    /// default.
    explicit StopTimer(const Str &name);

    /// Starts the timer (when stopped).
    void Start();

    /// Stops the timer (when running).
    void Stop();

    /// Stops if necessary and resets elapsed time to 0.
    void Reset();

    /// Gets the current elapsed time, whether stopped or not.
    double GetElapsedTime();

    /// Reports on the elapsed time to stdout.
    void Report();

  private:
    const Str   name_;
    bool        is_running_ = false;
    UTime       start_time_;
    double      elapsed_time_;
};

}  // namespace Debug
