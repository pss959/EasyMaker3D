#pragma once

#include <ostream>
#include <string>

#include "Math/Types.h"

//! \name Mesh Utilities
//!@{

//! Writes the given TriMesh in OFF format to the given stream. The description
//! is written as a comment. Note: OFF does not contain normals or texture
//! coords.
void WriteMeshAsOFF(const TriMesh &mesh, const std::string &description,
                    std::ostream &out);

//!@}
