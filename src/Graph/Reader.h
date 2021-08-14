#pragma once

class ExceptionBase;
class IResourceManager;

namespace Graph {

class Scene;

//! The Reader class reads a Scene from a file. Any failure results in a
//! Reader::Exception being thrown.
class Reader {
  public:
    //! Exception thrown when any loading function fails.
    typedef ExceptionBase Exception;

    //! The constructor is passed an IResourceManager that is used when reading
    //! resources, such as textures and shader source.
    Reader(IResourceManager &resource_manager);
    ~Reader();

    //! Reads a Scene from the path stored in the Scene, storing the results
    // back in the Scene instance.
    void ReadScene(Scene &scene);

  private:
    IResourceManager &resource_manager_;
};

}  // namespace Graph
