#pragma once

#include <string>

#include "IO/UnitConversion.h"
#include "Math/Types.h"
#include "Util/FilePath.h"

//! Reads the specified text or binary STL file, returning a TriMesh containing
//! the vertices and triangles. The UnitConversion is used to convert
//! coordinates. The mesh is obviously not guaranteed to be watertight. If
//! anything goes wrong, an empty TriMesh with no vertices or triangles is
//! returned and the error_message string will contain some useful information.
TriMesh ReadSTLFile(const Util::FilePath &path, const UnitConversion &conv,
                    std::string &error_message);
