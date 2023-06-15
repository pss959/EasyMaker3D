#pragma once

#include <filesystem>
#include <functional>
#include <ostream>
#include <string>

/// \file
/// This file contains classes and functions helping to deal with time.
///
/// \ingroup Utility

/// The UTime class wraps std::filesystem::file_time_type to make it easier to
/// use.
class UTime {
  public:
    /// The default constructor creates an undefined time.
    UTime() {}

    /// The constructor wraps a file_time_type.
    UTime(const std::filesystem::file_time_type &time);

    /// Constructs an instance representing the current time.
    static UTime Now();

    /// UTime comparisons.
    bool operator==(const UTime &other)  const = default;
    auto operator<=>(const UTime &other) const = default;

    /// Returns the duration from the given time to this one in seconds.
    double SecondsSince(const UTime &start) const;

    /// Allows output of UTime values.
    std::string ToString() const;

  private:
    /// Wrapped std::filesystem::file_time_type.
    std::filesystem::file_time_type time_;
};

/// Allows output of UTime values.
inline std::ostream & operator<<(std::ostream &out, const UTime &t) {
    return out << t.ToString();
}
