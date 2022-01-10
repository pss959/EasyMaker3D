#include "Util/UTime.h"

#include <chrono>
#include <future>
#include <thread>

#include <ion/base/stringutils.h>

UTime::UTime(const std::filesystem::file_time_type &time) : time_(time) {}

UTime UTime::Now() {
    return UTime(std::filesystem::file_time_type::clock::now());
}

double UTime::SecondsSince(const UTime &start) const {
    return std::chrono::duration<double>(time_ - start.time_).count();
}

std::string UTime::ToString() const {
    auto sctp =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            time_ - std::filesystem::file_time_type::clock::now()
            + std::chrono::system_clock::now());
    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
    // Remove the newline.
    return ion::base::TrimEndWhitespace(
        std::asctime(std::localtime(&cftime)));
}
