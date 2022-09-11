#pragma once

#include <string>

#include <ion/gfx/image.h>

class FilePath;

/// \file
/// This file contains utility functions for writing data to files.
///
/// \ingroup Utility

namespace Util {

/// Writes the given string to the given FilePath. Returns false if the file
/// could not be written.
bool WriteString(const FilePath &path, const std::string &s);

/// Writes an Ion Image to file with the given path. Returns false if the file
/// could not be written.
bool WriteImage(const FilePath &path, const ion::gfx::Image &image);

}  // namespace Util
