#pragma once

#include "Graph/Typedefs.h"
#include "Util/FilePath.h"

namespace Input {

class Tracker;

//! The Reader class reads a Graph::Scene from a file. Any failure results in
//! an Input::Exception being thrown.
//!
//! \ingroup Input
class Reader {
  public:
    //! The constructor is passed a Tracker instance that is used to track
    //! resources.
    Reader(Tracker &tracker);
    ~Reader();

    //! Reads and returns a Scene from the given path, which must be an
    //! absolute path.
    Graph::ScenePtr ReadScene(const Util::FilePath &path);

  private:
    Tracker &tracker_;
};

}  // namespace Input
