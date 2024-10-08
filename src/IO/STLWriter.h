//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@


#pragma once

#include <vector>

#include "Enums/FileFormat.h"

class FilePath;
struct TriMesh;

/// Writes the given meshes to an STL file in the given format. The conversion
/// factor is used to convert coordinates to millimeters, which is the standard
/// STL unit. Returns false on failure (file could not be opened for writing).
///
/// \ingroup IO
bool WriteSTLFile(const std::vector<TriMesh> &meshes, const FilePath &path,
                  FileFormat format, float conversion_factor);
