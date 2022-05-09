#pragma once

#include <string>

class FilePath;
class UnitConversion;
struct TriMesh;

/// Reads the specified text or binary STL file, returning a TriMesh containing
/// the vertices and triangles. The UnitConversion is used to convert
/// coordinates. The mesh is obviously not guaranteed to be watertight. If
/// anything goes wrong, an empty TriMesh with no vertices or triangles is
/// returned and the error_message string will contain some useful information.
///
/// \ingroup IO
TriMesh ReadSTLFile(const FilePath &path, const UnitConversion &conv,
                    std::string &error_message);
