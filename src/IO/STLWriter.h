#pragma once

#include <vector>

#include "Enums/FileFormat.h"
#include "Memory.h"

class FilePath;
class UnitConversion;
DECL_SHARED_PTR(Model);

/// Writes the given Models to an STL file in the given format. The
/// UnitConversion is used to convert coordinates.
///
/// \ingroup IO
void WriteSTLFile(const std::vector<ModelPtr> &models,
                  const FilePath &path, FileFormat format,
                  const UnitConversion &conv);
