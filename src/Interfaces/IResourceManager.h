#pragma once

#include <string>

#include <ion/gfx/image.h>
#include <ion/gfx/node.h>
#include <ion/gfxutils/shadermanager.h>

#include "Interfaces/IInterfaceBase.h"

//! Abstract Interface class for a resource manager, which locates and loads
//! resources from files. It guarantees that loading occurs only once unless
//! the resource file has been modified since last loaded. Each function takes
//! a relative path that is used to construct a full path relative to
//! RESOURCE_DIR.
//! \ingroup Interfaces
class IResourceManager : public IInterfaceBase {
  public:
    //! Returns the base path that all resources are relative to.
    virtual std::string GetBasePath() const = 0;

    //! Returns an Ion ShaderManager that can be used to create shader
    //! programs.
    virtual ion::gfxutils::ShaderManager & GetShaderManager() = 0;

    //! Loads Ion Node data from a file.
    virtual ion::gfx::NodePtr LoadNode(const std::string &path) = 0;

    //! Loads an Ion texture image from a file.
    virtual ion::gfx::ImagePtr LoadTextureImage(const std::string &path) = 0;

    //! Loads shader program source code from a file.
    virtual std::string LoadShaderSource(const std::string &path) = 0;

    //! Adds an external dependency between the given files.
    virtual void AddDependency(const std::string &owner_path,
                               const std::string &dep_path) = 0;
};
