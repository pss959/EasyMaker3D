#pragma once

#include <string>

#include <ion/gfx/image.h>
#include <ion/gfx/shape.h>

#include "Util/FilePath.h"

/// \file
/// This file contains utility functions for reading data from files.
///
/// \ingroup Utility

namespace Util {

/// Reads the contents of the file with the given path into a string and stores
/// it in s. Returns false if the file could not be read. If allow_includes is
/// true, this replaces any string of the form:
///
///   \code @include "path/to/file" \endcode
///
/// anywhere in the file with the contents of reading that file, which is
/// relative to the containing file's path.
bool ReadFile(const FilePath &path, std::string &s,
              bool allow_includes = false);

/// Reads the contents of an image file with the given path and returns an Ion
/// Image representing it. Returns a null pointer if the file could not be
/// read. If flip_vertically is true, the resulting image is flipped
/// vertically.
ion::gfx::ImagePtr ReadImage(const FilePath &path, bool flip_vertically);

/// Reads the contents of a shape file with the given path and returns an Ion
/// Shape representing it. Returns a null pointer if the file could not be
/// read. Sets up for normals and texture coordinate if requested.
ion::gfx::ShapePtr ReadShape(const FilePath &path,
                             bool use_normals, bool use_tex_coords);

}  // namespace Util
