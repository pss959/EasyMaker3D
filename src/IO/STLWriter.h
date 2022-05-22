#pragma once

#include <vector>

#include "Enums/FileFormat.h"

class FilePath;
class Selection;

/// Writes the selected Models to an STL file in the given format. The
/// conversion factor is used to convert coordinates. Returns false on failure
/// (file could not be opened for writing).
///
/// \ingroup IO
bool WriteSTLFile(const Selection &sel, const FilePath &path,
                  FileFormat format, float conversion_factor);
