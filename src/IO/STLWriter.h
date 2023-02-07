#pragma once

#include <vector>

#include "Enums/FileFormat.h"

class FilePath;
class Selection;
struct TriMesh;

/// Writes the given meshes to an STL file in the given format. The conversion
/// factor is used to convert coordinates to millimeters, which is the standard
/// STL unit. Returns false on failure (file could not be opened for writing).
///
/// \ingroup IO
bool WriteSTLFile(const std::vector<TriMesh> &meshes, const FilePath &path,
                  FileFormat format, float conversion_factor);
