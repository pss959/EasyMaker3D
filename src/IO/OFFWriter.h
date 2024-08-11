//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

struct Color;
class  FilePath;
struct TriMesh;

/// Writes the given meshes to an OFF file. If the colors vector is not empty,
/// the colors in it are used per mesh; if not enough are specified, the colors
/// are cycled through again. Returns false on failure (file could not be
/// opened for writing).
///
/// \ingroup IO
bool WriteOFFFile(const std::vector<TriMesh> &meshes,
                  const std::vector<Color> &colors, const FilePath &path);
