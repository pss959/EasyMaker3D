#include "Util/Time.h"

#include <chrono>

#include <ion/base/stringutils.h>

namespace Util {

Time::Time(const std::filesystem::file_time_type &time) : time_(time) {}

Time Time::Now() {
    return Time(std::filesystem::file_time_type::clock::now());
}

double Time::SecondsSince(const Time &start) const {
    return std::chrono::duration<double>(time_ - start.time_).count();
}

std::string Time::ToString() const {
    auto sctp =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            time_ - std::filesystem::file_time_type::clock::now()
            + std::chrono::system_clock::now());
    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
    // Remove the newline.
    return ion::base::TrimEndWhitespace(
        std::asctime(std::localtime(&cftime)));
}

}  // namespace Util
