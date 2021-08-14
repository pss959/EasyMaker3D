#pragma once

#include <string>

#include <ion/gfx/image.h>
#include <ion/gfx/node.h>
#include <ion/gfxutils/shadermanager.h>

#include "Interfaces/IInterfaceBase.h"
#include "Util/FilePath.h"

//! Abstract Interface class for a resource manager, which stores resources
//! loaded from files. It can be used to guarantee that loading occurs only
//! once unless the resource file has been modified since last loaded. Each
//! function takes a Util::FilePath that is relative to RESOURCE_DIR.
//!
//! \ingroup //! Interfaces
class IResourceManager : public IInterfaceBase {
  public:
    //! Convenience typedef for a path.
    typedef Util::FilePath Path;

    //! Returns the base path that all resources are relative to.
    virtual Path GetBasePath() const = 0;

    //! Returns an Ion ShaderManager that can be used to create shader
    //! programs.
    virtual ion::gfxutils::ShaderManager & GetShaderManager() = 0;

    //! \name Adding Resources
    //! Each of these functions adds a resource of a given type to the manager,
    //! associating it with a file path. This sets the load time for the
    //! resource to the current time.
    //!@{
    virtual void AddTextureImage(const Path &path,
                                 const ion::gfx::ImagePtr &image) = 0;
    virtual void AddShaderSource(const Path &path,
                                 const std::string &source) = 0;
    //!@}

    //! \name Finding Resources
    //! Each of these functions looks for an added resource of a given type
    //! that is associated with a file path. If none is found, or if one is
    //! found but its file has changed since it was loaded, each returns a null
    //! pointer or empty string.
    //!@{
    virtual ion::gfx::ImagePtr FindTextureImage(const Path &path) = 0;
    virtual std::string FindShaderSource(const Path &path) = 0;
    //!@}

    //! Adds an external dependency between the given files so that the owner
    //! will be marked as out of date if the dependency is modified.
    virtual void AddDependency(const Path &owner_path,
                               const Path &dep_path) = 0;
};
