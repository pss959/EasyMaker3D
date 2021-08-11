#pragma once

#include <string>

#include <ion/gfx/image.h>
#include <ion/gfx/node.h>

class ExceptionBase;
class IResourceManager;

//! The Loader class loads resources of different types from files, returning
//! an Ion pointer to the results. Failure of any loading function results in a
//! Loader::Exception being thrown.
class Loader {
  public:
    //! Exception thrown when any loading function fails.
    typedef ExceptionBase Exception;

    //! The constructor is passed an IResourceManager that is used when loading
    //! resources, such as textures and shader source.
    Loader(IResourceManager &resource_manager);
    ~Loader();

    //! Loads the contents of the file with the given path into a string and
    //! returns it.
    std::string LoadFile(const std::string &path);

    //! Loads an Ion node subgraph from the file with the given path, returning
    //! an Ion NodePtr to the result.
    ion::gfx::NodePtr LoadNode(const std::string &path);

    //! Loads an image the file with the given path, returning an Ion ImagePtr
    //! to the result.
    ion::gfx::ImagePtr LoadImage(const std::string &path);

  private:
    IResourceManager &resource_manager_;
};
