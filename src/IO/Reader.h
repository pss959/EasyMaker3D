#pragma once

#include "SG/Tracker.h"
#include "SG/Typedefs.h"
#include "Util/FilePath.h"

/// The Reader class reads an SG::Scene from a file. Any failure results in a
/// Parser::Exception or SG::Exception being thrown.
class Reader {
  public:
    /// Reads and returns a Scene from the given path, which must be an
    /// absolute path. The Tracker is used to manage dependencies.
    SG::ScenePtr ReadScene(const Util::FilePath &path, SG::Tracker &tracker);
};
