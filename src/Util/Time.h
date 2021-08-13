#pragma once

#include <assert.h>

#include <chrono>
#include <filesystem>
#include <memory>
#include <ostream>
#include <string>

namespace Util {

//! \name Time Helpers
//!@{

//! The Time class wraps std::filesystem::file_time_type to make it easier to
//! use.
class Time {
  public:
    //! The default constructor creates an undefined time.
    Time() {}

    //! The constructor wraps a file_time_type.
    Time(const std::filesystem::file_time_type &time) : time_(time) {}

    //! Constructs an instance representing the current time.
    static Time Now() {
        return Time(std::filesystem::file_time_type::clock::now());
    }

    //! Constructs an instance representing the last modification time of a
    //! file with the given path, which must exist.
    static Time ModTime(const std::string &path) {
        assert(std::filesystem::exists(path));
        return Time(std::filesystem::last_write_time(path));
    }

    //! Time comparisons.
    bool operator==(const Time &other) { return time_ == other.time_; }
    bool operator!=(const Time &other) { return time_ != other.time_; }
    bool operator<(const  Time &other) { return time_ <  other.time_; }
    bool operator>(const  Time &other) { return time_ >  other.time_; }
    bool operator<=(const Time &other) { return time_ <= other.time_; }
    bool operator>=(const Time &other) { return time_ >= other.time_; }

    //! Allows output of Time values.
    std::string ToString() const {
        auto sctp =
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                time_ - std::filesystem::file_time_type::clock::now()
                + std::chrono::system_clock::now());
        std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
        return std::asctime(std::localtime(&cftime));
    }

  private:
    //! Wrapped std::filesystem::file_time_type.
    std::filesystem::file_time_type time_;
};

//! Allows output of Time values.
inline std::ostream & operator<<(std::ostream &out, const Time &t) {
    return out << t.ToString();
}

//!@}

}  // namespace Util
