#pragma once

#include <iostream>
#include <string>

/// \file
/// This file contains facilities for selectively logging messages to help with
/// debugging. Messages are printed only if a supplied key is part of an
/// app-defined key string. None of this is enabled in a non-debug build.
///
/// \ingroup Utility

/// The KLogger class prints a message to std::cout if the message key (a
/// character) is present in the current key string.
///
/// \ref KLogCodes "The full list of character codes can be found here."
///
/// \ingroup Utility
class KLogger : public std::ostream {
  public:
    /// Sets a string containing all active character keys: only these keys
    /// will cause printing in subsequent log messages.
    static void SetKeyString(const Str &key_string) {
        key_string_ = key_string;
    }

    /// Appends to the string set by SetKeyString().
    static void AppendKeyString(const Str &key_string) {
        key_string_ += key_string;
    }

    /// Returns true if the given key is present in the key string.
    static bool HasKeyCharacter(char key);

    /// Toggles logging by adding or removing a '!' character to the string.
    static void ToggleLogging();

    /// Sets the current render count to use in log messages.
    static void SetRenderCount(size_t count) { render_count_ = count; }

    KLogger(char key);

    /// Returns the stream to which output is sent.
    std::ostream & GetStream();

  private:
    /// A message key must be present in this string for the message to appear.
    static Str    key_string_;

    /// Current render count for log messages.
    static size_t render_count_;

    /// This will be true if the message should be printed.
    const bool    do_print_;

    /// Returns true if printing should occur for the given key character.
    static bool ShouldPrint_(char key);
};

/// The KLOG macro logs the streamed message if logging is enabled and the
/// given key is part of the current key string.
#if ENABLE_DEBUG_FEATURES
#  define KLOG(key, msg) KLogger(key).GetStream() << msg << "\n"
#else
#  define KLOG(key, msg)
#endif
