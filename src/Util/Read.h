#pragma once

#include <string>

#include <ion/gfx/image.h>
#include <ion/gfx/shape.h>

#include "Util/FilePath.h"

namespace Util {

//! \name File Reading Utilities
//!@{

//! Reads the contents of the file with the given path into a string and stores
//! it in s. Returns false if the file could not be read.
bool ReadFile(const FilePath &path, std::string &s);

//! Reads the contents of an image file with the given path and returns an Ion
//! Image representing it. Returns a null pointer if the file could not be
//! read.
ion::gfx::ImagePtr ReadImage(const FilePath &path);

//! Reads the contents of a shape file with the given path and returns an Ion
//! Shape representing it. Returns a null pointer if the file could not be
//! read.
ion::gfx::ShapePtr ReadShape(const FilePath &path);

//!@}

}  // namespace Util
