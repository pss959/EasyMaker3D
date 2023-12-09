#include "Debug/Timer.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

#include "Math/ToString.h"
#include "Util/Assert.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

struct Duration_ {
    const UTime &t0;
    const UTime &t1;
    Duration_(const UTime &t0_in, const UTime &t1_in) : t0(t0_in), t1(t1_in) {}
};

std::ostream & operator<<(std::ostream &out, const Duration_ &d) {
    out << std::setw(8) << Math::ToString(d.t1.SecondsSince(d.t0), .0001);
    return out;
}

}  // anonymous namespace

namespace Debug {

// ----------------------------------------------------------------------------
// Timer functions.
// ----------------------------------------------------------------------------

Timer::Timer(const Str &name) : name_(name) {
    // Avoid unnecessary allocations.
    timepoints_.reserve(100);
    Reset();
}

void Timer::Reset() {
    reset_time_ = UTime::Now();
    timepoints_.clear();
}

void Timer::AddTimePoint(const Str &name) {
    timepoints_.emplace_back(name, UTime::Now());
}

void Timer::Report() {
    std::cout << "=== Timer '" << name_ << "'\n";

    size_t max_len = 0;
    for (const auto &tp: timepoints_)
        max_len = std::max(max_len, tp.name.size());

    UTime end_time = UTime::Now();
    UTime prev_time = reset_time_;
    for (const auto &tp: timepoints_) {
        std::cout << "  " << std::setw(max_len) << tp.name
                  << ": "       << Duration_(prev_time,   tp.time)
                  << "   cum: " << Duration_(reset_time_, tp.time)
                  << "\n";
        prev_time = tp.time;
    }
    std::cout << "  TOTAL: "
              << Math::ToString(end_time.SecondsSince(reset_time_), .0001)
              << "\n";
}

double Timer::GetTotalTime() {
    return UTime::Now().SecondsSince(reset_time_);
}

// ----------------------------------------------------------------------------
// StopTimer functions.
// ----------------------------------------------------------------------------

StopTimer::StopTimer(const Str &name) : name_(name) {
    Reset();
}

void StopTimer::Start() {
    ASSERT(! is_running_);
    is_running_ = true;
    start_time_ = UTime::Now();
}

void StopTimer::Stop() {
    ASSERT(is_running_);
    elapsed_time_ += UTime::Now().SecondsSince(start_time_);
    is_running_ = false;
}

void StopTimer::Reset() {
    is_running_   = false;
    elapsed_time_ = 0;
}

double StopTimer::GetElapsedTime() {
    if (is_running_)
        return elapsed_time_ + UTime::Now().SecondsSince(start_time_);
    else
        return elapsed_time_;
}

void StopTimer::Report() {
    std::cout << "=== StopTimer '" << name_ << "': elapsed_time = "
              << Math::ToString(elapsed_time_, .0001) << "\n";
}

}  // namespace Debug
