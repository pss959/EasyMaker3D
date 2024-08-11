//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

class FilePath;
struct TriMesh;

/// Reads the specified text or binary STL file, returning a TriMesh containing
/// the vertices and triangles. The conversion factor is used to convert
/// coordinates. The mesh is obviously not guaranteed to be watertight. If
/// anything goes wrong, an empty TriMesh with no vertices or triangles is
/// returned and the error_message string will contain some useful information.
///
/// \ingroup IO
TriMesh ReadSTLFile(const FilePath &path, float conversion_factor,
                    Str &error_message);
