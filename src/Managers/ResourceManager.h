#pragma once

#include <memory>
#include <string>

#include "Interfaces/IResourceManager.h"

//! ResourceManager implements the IResourceManager interface, using a Loader
//! instance to do most of the loading work.
// \ingroup Managers
class ResourceManager : public IResourceManager {
  public:
    ResourceManager();
    ~ResourceManager();

    virtual const char * GetClassName() const override {
        return "ResourceManager";
    }
    virtual std::string GetBasePath() const override {
        return RESOURCE_DIR;
    }
    virtual ion::gfxutils::ShaderManager & GetShaderManager() override;
    virtual ion::gfx::NodePtr LoadNode(const std::string &path) override;
    virtual ion::gfx::ImagePtr LoadTextureImage(
        const std::string &path) override;
    virtual std::string LoadShaderSource(const std::string &path) override;
    virtual void AddDependency(const std::string &owner_path,
                               const std::string &dep_path) override;

  private:
    class Tracker_;

    //! Ion ShaderManager used for creating shaders.
    ion::gfxutils::ShaderManagerPtr shader_manager_;

    //! Tracker_ that manages resource tracking.
    std::unique_ptr<Tracker_> tracker_;

    //! Returns a full path for a file..
    static std::string GetPath_(const std::string &type_name,
                                const std::string &path);
};
