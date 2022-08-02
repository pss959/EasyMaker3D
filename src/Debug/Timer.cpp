#include "Debug/Timer.h"

#include <iomanip>
#include <iostream>

#include "Math/ToString.h"

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

// ----------------------------------------------------------------------------
// Timer functions.
// ----------------------------------------------------------------------------

namespace Debug {

Timer::Timer(const std::string &name) : name_(name) {
    // Avoid unnecessary allocations.
    timepoints_.reserve(100);
}

void Timer::Reset() {
    reset_time_ = UTime::Now();
    timepoints_.clear();
}

void Timer::AddTimePoint(const std::string &name) {
    timepoints_.emplace_back(name, UTime::Now());
}

void Timer::Report() {
    std::cout << "=== Timer '" << name_ << "'\n";

    UTime end_time = UTime::Now();
    UTime prev_time = reset_time_;
    for (const auto &tp: timepoints_) {
        std::cout << "  " << tp.name
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

}  // namespace Debug
