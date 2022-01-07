#pragma once

#include <iostream>
#include <string>

/// \file
/// This file contains facilities for selectively logging messages to help with
/// debugging. Messages are printed only if a supplied key is part of an
/// app-defined key string. None of this is enabled in a non-debug build.

/// The KLogger class prints a message to std::cout if the message key (a
/// character) is present in the current key string.
class KLogger : public std::ostream {
  public:
    /// Sets a string containing all active character keys: only these keys
    /// will cause printing in subsequent log messages.
    static void SetKeyString(const std::string &key_string) {
        key_string_ = key_string;
    }

    /// Appends to the string set by SetKeyString().
    static void AppendKeyString(const std::string &key_string) {
        key_string_ += key_string;
    }

    /// Returns true if the given key is present in the key string.
    static bool HasKeyCharacter(char key);

    KLogger(char key);

    /// Returns the stream to which output is sent.
    std::ostream & GetStream();

  private:
    // A message key must be present in this string for the message to appear.
    static std::string key_string_;

    // This will be true if the message should be printed.
    const bool do_print_;
};

/// The KLOG macro logs the streamed message if logging is enabled and the
/// given key is part of the current key string.
#if ENABLE_LOGGING
#  define KLOG(key, msg) KLogger(key).GetStream() << msg << "\n"
#else
#  define KLOG(key, msg)
#endif
