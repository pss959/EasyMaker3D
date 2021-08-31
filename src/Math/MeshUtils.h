#pragma once

#include <ostream>
#include <string>

#include "SG/Math.h"

namespace Math {

//! \name Mesh Utilities
//!@{

//! Writes the given TriMesh in OFF format to the given stream. The description
//! is written as a comment.
void WriteMeshAsOFF(const SG::TriMesh &mesh, const std::string &description,
                    std::ostream &out);

//!@}

}  // namespace Util
