#pragma once

#include <ostream>
#include <string>

#include "Math/Types.h"

//! \name Mesh Utilities
//!@{

//! Writes the given TriMesh in OFF format to the given stream. The description
//! is written as a comment.
void WriteMeshAsOFF(const TriMesh &mesh, const std::string &description,
                    std::ostream &out);

//!@}
