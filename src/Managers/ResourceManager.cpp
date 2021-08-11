#include "Managers/ResourceManager.h"

#include <filesystem>
#include <unordered_map>

#include "Loader.h"
#include "Util.h"

using ion::gfx::ImagePtr;
using ion::gfx::NodePtr;

// ----------------------------------------------------------------------------
// ResourceManager::Tracker_ class definition.
// ----------------------------------------------------------------------------

//! ResourceManager::Tracker_ is an internal class used to track which
//! resources have been loaded, and when.
class ResourceManager::Tracker_ {
  public:
    //! Adds a node associated with a path.
    void AddNode(const std::string &path, NodePtr node) {
        node_map_[path] = BuildFileData_<NodePtr>(path, node);
    }

    //! Adds a texture image associated with a path.
    void AddTextureImage(const std::string &path, ImagePtr image) {
        texture_map_[path] = BuildFileData_<ImagePtr>(path, image);
    }

    //! Adds a shader source string associated with a path.
    void AddShaderSource(const std::string &path, const std::string &source) {
        shader_map_[path] = BuildFileData_<std::string>(path, source);
    }

    //! Returns the node with the given path. If the path was not added or if
    //! the file changed since it was read, this returns a null NodePtr.
    NodePtr FindNode(const std::string &path) {
        return FindItem_<NodePtr>(path, node_map_);
    }

    //! Returns the texture image with the given path. If the path was not
    //! added or if the file changed since it was read, this returns a null
    //! ImagePtr.
    ImagePtr FindImage(const std::string &path) {
        return FindItem_<ImagePtr>(path, texture_map_);
    }

    //! Returns the shader source with the given path. If the path was not
    //! added or if the file changed since it was read, this returns an empty
    //! string.
    std::string FindShaderSource(const std::string &path) {
        return FindItem_<std::string>(path, shader_map_);
    }

  private:
    //! Struct storing the resource associated with a file path and the last
    //! modification time of the file.
    template <typename T> struct FileData_ {
        //! Path to the file.
        std::string  path;
        //! Time when the file was last loaded.
        Util::Time   load_time;
        //! The data loaded from the file.
        T            data;

        //! Returns true if the FileData_ is still valid, meaning that the file
        //! was not removed or modified since it was loaded.
        bool IsValid() const {
            return std::filesystem::exists(path) &&
                Util::Time(std::filesystem::last_write_time(path)) <= load_time;
        }
    };

    //! Typedef for map for node storage.
    typedef std::unordered_map<std::string, FileData_<NodePtr>>     NodeMap_;
    //! Typedef for map for texture image storage.
    typedef std::unordered_map<std::string, FileData_<ImagePtr>>    TextureMap_;
    //! Typedef for map for shader source storage.
    typedef std::unordered_map<std::string, FileData_<std::string>> ShaderMap_;

    //! Maps file path for a Node to its FileData_.
    NodeMap_    node_map_;
    //! Maps file path for a texture Image to its FileData_.
    TextureMap_ texture_map_;
    //! Maps file path for a shader program to its FileData_.
    ShaderMap_  shader_map_;

    //! Builds and returns a FileData_ of the given type.
    template <typename T> FileData_<T> BuildFileData_(const std::string &path,
                                                      const T &t) {
        FileData_<T> data;
        data.path      = path;
        data.load_time = std::filesystem::file_time_type::clock::now();
        data.data      = t;
        return data;
    }

    //! Implements the Find function for the given type.
    template <typename T> T FindItem_(
        const std::string &path,
        std::unordered_map<std::string, FileData_<T>> &map) {
        auto it = map.find(path);
        if (it != map.end()) {
            // If the item is still valid, return it.
            if (it->second.IsValid())
                return it->second.data;
            // Otherwise, need to reload and add it again.
            map.erase(it);
        }
        return T();
    }
};

ResourceManager::ResourceManager() :
    shader_manager_(new ion::gfxutils::ShaderManager),
    tracker_(new Tracker_) {
}

ResourceManager::~ResourceManager() {
}

ion::gfxutils::ShaderManager & ResourceManager::GetShaderManager() {
    return *shader_manager_;
}

NodePtr ResourceManager::LoadNode(const std::string &path) {
    const std::string full_path = GetPath_("nodes", path);
    NodePtr node = tracker_->FindNode(full_path);
    if (! node) {
        std::cerr << "XXXX Loading node from '" << full_path << "'\n";
        node = Loader(*this).LoadNode(full_path);
        tracker_->AddNode(full_path, node);
    }
    return node;
}

ImagePtr ResourceManager::LoadTextureImage(const std::string &path) {
    const std::string full_path = GetPath_("textures", path);
    ImagePtr image = tracker_->FindImage(full_path);
    if (! image) {
        std::cerr << "XXXX Loading texture from '" << full_path << "'\n";
        image = Loader(*this).LoadImage(full_path);
        tracker_->AddTextureImage(full_path, image);
    }
    return image;
}

std::string ResourceManager::LoadShaderSource(const std::string &path) {
    const std::string full_path = GetPath_("shaders", path);
    std::string source = tracker_->FindShaderSource(full_path);
    if (source.empty()) {
        std::cerr << "XXXX Loading shader from '" << full_path << "'\n";
        source = Loader(*this).LoadFile(full_path);
        // Make sure the source is not empty, which signals error.
        if (source.empty())
            source = " ";
        tracker_->AddShaderSource(full_path, source);
    }
    return source;
}

std::string ResourceManager::GetPath_(const std::string &type_name,
                                      const std::string &path) {
    std::filesystem::path full_path(RESOURCE_DIR);
    full_path /= type_name;
    full_path /= path;
    return full_path.native();
}
