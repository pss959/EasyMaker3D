#include "Util/Time.h"

#include <chrono>
#include <future>
#include <thread>

#include <ion/base/stringutils.h>

namespace Util {

static int delay_count_ = 0;

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

void Delay(float seconds, const std::function<void()> &func) {
    ++delay_count_;
    std::thread delay_thread([seconds, func]() {
        std::this_thread::sleep_for(std::chrono::duration<float>(seconds));
        func();
        --delay_count_;  // TODO: Use mutex for this if necessary.
    });
    delay_thread.detach();
}

bool IsDelaying() {
    return delay_count_ > 0;
}

}  // namespace Util
