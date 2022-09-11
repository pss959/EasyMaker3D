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
/// could not be written. If flip_vertically is true, the written image is
/// flipped vertically.
bool WriteImage(const FilePath &path, const ion::gfx::Image &image,
                bool flip_vertically);

}  // namespace Util
