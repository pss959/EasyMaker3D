#pragma once

#include <filesystem>
#include <unordered_map>
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
    virtual ion::gfxutils::ShaderManager & GetShaderManager() override;
    virtual ion::gfx::NodePtr LoadNode(const std::string &path) override;
    virtual ion::gfx::ImagePtr LoadTextureImage(
        const std::string &path) override;
    virtual std::string LoadShaderSource(const std::string &path) override;

  private:
    //! Struct storing the resource associated with a file path and the last
    //! modification time of the file.
    template <typename T> struct FileData_ {
        //! Time when the file was last loaded.
        std::filesystem::file_time_type load_time;
        //! The data loaded from the file.
        T                               data;
    };

    //! Typedef for data for node storage.
    typedef FileData_<ion::gfx::NodePtr>  NodeData_;
    //! Typedef for data for texture image storage.
    typedef FileData_<ion::gfx::ImagePtr> TextureData_;
    //! Typedef for data for shader source storage.
    typedef FileData_<std::string>        ShaderData_;

    //! Typedef for map for node storage.
    typedef std::unordered_map<std::string, NodeData_>    NodeMap_;
    //! Typedef for map for texture image storage.
    typedef std::unordered_map<std::string, TextureData_> TextureMap_;
    //! Typedef for map for shader source storage.
    typedef std::unordered_map<std::string, ShaderData_>  ShaderMap_;

    //! Maps file path for a Node to its FileData_.
    NodeMap_    node_map_;
    //! Maps file path for a texture Image to its FileData_.
    TextureMap_ texture_map_;
    //! Maps file path for a shader program to its FileData_.
    ShaderMap_  shader_map_;

    //! Ion ShaderManager used for creating shaders.
    ion::gfxutils::ShaderManagerPtr shader_manager_;

    //! Returns a full path for a file..
    static std::string GetPath_(const std::string &type_name,
                                const std::string &path);
};
