#pragma once

#include "Graph/Typedefs.h"
#include "Util/FilePath.h"

#include <ion/gfxutils/shadermanager.h>

namespace Input {

class Tracker;

//! The Reader class reads a Graph::Scene from a file. Any failure results in
//! an Input::Exception being thrown.
//!
//! \ingroup Input
class Reader {
  public:
    //! The constructor is passed a Tracker instance that is used to track
    //! resources and an Ion ShaderManager used to create shaders.
    Reader(Tracker &tracker, ion::gfxutils::ShaderManager &shader_manager);
    ~Reader();

    //! Reads and returns a Scene from the given path, which must be an
    //! absolute path.
    Graph::ScenePtr ReadScene(const Util::FilePath &path);

  private:
    //! Tracker instance used to track resources to avoid extra loading.
    Tracker &tracker_;

    //! ShaderManager used to create shaders.
    ion::gfxutils::ShaderManager &shader_manager_;
};

}  // namespace Input
