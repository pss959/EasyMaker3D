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
    //! Alias for external loading function.
    template <typename T>
    using LoadFunc = std::function<T(const std::string &)>;

    NodePtr GetNode(const std::string &path,
                    const LoadFunc<NodePtr> load_func) {
        return GetItem_<NodePtr>(path, "node", node_map_, load_func);
    }

    ImagePtr GetTextureImage(const std::string &path,
                             const LoadFunc<ImagePtr> load_func) {
        return GetItem_<ImagePtr>(path, "texture", texture_map_, load_func);
    }

    std::string GetShaderSource(const std::string &path,
                                const LoadFunc<std::string> load_func) {
        return GetItem_<std::string>(path, "shader", shader_map_, load_func);
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

    //! Alias for a map from a path to a FileData_ instance of the templated
    //! type..
    template <typename T>
    using Map_ = std::unordered_map<std::string, FileData_<T>>;

    //! Maps file path for a Node to its FileData_.
    Map_<NodePtr>     node_map_;
    //! Maps file path for a texture Image to its FileData_.
    Map_<ImagePtr>    texture_map_;
    //! Maps file path for a shader program to its FileData_.
    Map_<std::string> shader_map_;

    //! Accesses the item with the given path from the map, returning it if it
    //! has been added and is still valid. Otherwise, calls the load function
    //! to load the data and stores the result in the map.
    template <typename T> T GetItem_(
        const std::string &path, const std::string &item_type, Map_<T> &map,
        const LoadFunc<T> &load_func) {
        T item = FindItem_<T>(path, map);
        if (item == T()) {
            std::cerr << "XXXX Loading " << item_type
                      << " from '" << path << "'\n";
            item = load_func(path);
            map[path] = BuildFileData_<T>(path, item);
        }
        return item;
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

    //! Builds and returns a FileData_ of the given type.
    template <typename T> FileData_<T> BuildFileData_(const std::string &path,
                                                      const T &t) {
        FileData_<T> data;
        data.path      = path;
        data.load_time = std::filesystem::file_time_type::clock::now();
        data.data      = t;
        return data;
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
    auto loader_func =
        [this](const std::string &p){ return Loader(*this).LoadNode(p); };
    return tracker_->GetNode(GetPath_("nodes", path), loader_func);
}

ImagePtr ResourceManager::LoadTextureImage(const std::string &path) {
    auto loader_func =
        [this](const std::string &p){ return Loader(*this).LoadImage(p); };
    return tracker_->GetTextureImage(GetPath_("textures", path), loader_func);
}

std::string ResourceManager::LoadShaderSource(const std::string &path) {
    auto loader_func =
        [this](const std::string &p){ return Loader(*this).LoadFile(p); };
    return tracker_->GetShaderSource(GetPath_("shaders", path), loader_func);
}

std::string ResourceManager::GetPath_(const std::string &type_name,
                                      const std::string &path) {
    std::filesystem::path full_path(RESOURCE_DIR);
    full_path /= type_name;
    full_path /= path;
    return full_path.native();
}
