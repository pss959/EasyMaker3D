#pragma once

class ExceptionBase;
class IResourceManager;

namespace Input {

class Scene;

//! The Reader class reads a Scene from a file. Any failure results in a
//! Reader::Exception being thrown.
//!
//! \ingroup Input
class Reader {
  public:
    //! Exception thrown when any loading function fails.
    typedef ExceptionBase Exception;

    //! The constructor is passed an IResourceManager that is used when reading
    //! resources, such as textures and shader source.
    Reader(IResourceManager &resource_manager);
    ~Reader();

    //! Reads and returns a Scene from the given path.
    Graph::ScenePtr ReadScene(const Util::FilePath &path);

  private:
    IResourceManager &resource_manager_;
};

}  // namespace Input
