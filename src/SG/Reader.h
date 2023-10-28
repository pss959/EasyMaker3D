#pragma once

#include "Util/FilePath.h"
#include "Util/Memory.h"

namespace SG {

class FileMap;
DECL_SHARED_PTR(Scene);

/// The Reader class reads an SG::Scene from a file. Any failure results in a
/// Parser::Exception or SG::Exception being thrown.
///
/// \ingroup SG
class Reader {
  public:
    /// Reads and returns a Scene from the given path, which must be an
    /// absolute path. The FileMap is used to manage dependencies.
    ScenePtr ReadScene(const FilePath &path, FileMap &file_map);
};

}  // namespace SG
