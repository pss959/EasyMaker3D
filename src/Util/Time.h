#pragma once

#include <filesystem>
#include <functional>
#include <ostream>
#include <string>

namespace Util {

/// \name Time Helpers
///@{

/// The Time class wraps std::filesystem::file_time_type to make it easier to
/// use.
class Time {
  public:
    /// The default constructor creates an undefined time.
    Time() {}

    /// The constructor wraps a file_time_type.
    Time(const std::filesystem::file_time_type &time);

    /// Constructs an instance representing the current time.
    static Time Now();

    /// Time comparisons.
    bool operator==(const Time &other) { return time_ == other.time_; }
    bool operator!=(const Time &other) { return time_ != other.time_; }
    bool operator<(const  Time &other) { return time_ <  other.time_; }
    bool operator>(const  Time &other) { return time_ >  other.time_; }
    bool operator<=(const Time &other) { return time_ <= other.time_; }
    bool operator>=(const Time &other) { return time_ >= other.time_; }

    /// Returns the duration from the given time to this one in seconds.
    double SecondsSince(const Time &start) const;

    /// Allows output of Time values.
    std::string ToString() const;

  private:
    /// Wrapped std::filesystem::file_time_type.
    std::filesystem::file_time_type time_;
};

/// Allows output of Time values.
inline std::ostream & operator<<(std::ostream &out, const Time &t) {
    return out << t.ToString();
}

/// Executes a function after a given delay, in seconds. This operates
/// asynchronously so that the main thread does not block.
void Delay(float seconds, const std::function<void()> &func);

/// Returns true if any delay is in effect.
bool IsDelaying();

///@}

}  // namespace Util
