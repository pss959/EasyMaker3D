#pragma once

#include <memory>

#include "Interfaces/IResourceManager.h"

//! ResourceManager implements the IResourceManager interface, using a Loader
//! instance to do most of the loading work.
//!
// \ingroup Managers
class ResourceManager : public IResourceManager {
  public:
    ResourceManager();
    ~ResourceManager();

    virtual const char * GetClassName() const override {
        return "ResourceManager";
    }
    virtual Path GetBasePath() const override {
        return RESOURCE_DIR;
    }
    virtual ion::gfxutils::ShaderManager & GetShaderManager() {
        return *shader_manager_;
    }
    virtual void AddNode(const Path &path,
                         const ion::gfx::NodePtr &node) override;
    virtual void AddTextureImage(const Path &path,
                                 const ion::gfx::ImagePtr &image) override;
    virtual void AddShaderSource(const Path &path,
                                 const std::string &source) override;
    virtual ion::gfx::NodePtr FindNode(const Path &path) override;
    virtual ion::gfx::ImagePtr FindTextureImage(const Path &path) override;
    virtual std::string FindShaderSource(const Path &path) override;
    virtual void AddDependency(const Path &owner_path,
                               const Path &dep_path) override;

  private:
    class Tracker_;

    //! Ion ShaderManager used for creating shaders.
    ion::gfxutils::ShaderManagerPtr shader_manager_;

    //! Tracker_ that manages resource tracking.
    std::unique_ptr<Tracker_> tracker_;

    //! Returns a full path for a file..
    static Path GetFullPath_(const std::string &type_name, const Path &path);
};
