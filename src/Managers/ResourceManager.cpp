#include "Managers/ResourceManager.h"

#include <filesystem>
#include <functional>
#include <stack>
#include <unordered_map>

#include "Loader.h"
#include "Util/Time.h"

using ion::gfx::ImagePtr;
using ion::gfx::NodePtr;

//! Convenient alias for map from file path to an item.
template <typename T> using Map_ = std::unordered_map<std::string, T>;

// ----------------------------------------------------------------------------
// Internal Dependency tracking class.
// ----------------------------------------------------------------------------

//! The DependencyTracker_ class tracks dependencies between files specified by
//! their paths. Each file is stored along with a time (Util::Time) at which it
//! was last accessed. The times are used to determine whether loaded file data
//! is still valid.
class DependencyTracker_ {
  public:
    //! Adds the load time for the given path using the current time.
    void AddLoadTime(const std::string &path) {
        load_time_map_[path] = Util::Time::Now();
    }

    //! Adds a dependency between the given paths.
    void AddDependency(const std::string &owner_path,
                       const std::string &dep_path) {
        auto it = dep_map_.find(owner_path);
        if (it == dep_map_.end())
            dep_map_[owner_path] = std::vector<std::string>(1, dep_path);
        else
            it->second.push_back(dep_path);
    }

    //! Returns true if data for the given path is still known to be valid,
    //! meaning that its file has not been modified since it was loaded and
    //! that all dependencies are also still valid. If the path is no longer
    //! valid, it is removed from the DependencyTracker_.
    bool IsValid(const std::string &path) {
        // Make sure the path still exists.
        if (std::filesystem::exists(path)) {
            // Get the load time for the path.
            auto it = load_time_map_.find(path);
            if (it != load_time_map_.end()) {
                const Util::Time &load_time = it->second;

                if (Util::Time::ModTime(path) <= load_time &&
                    AreDependenciesValid_(path))
                    return true;
            }
        }

        // No longer valid. It will be reloaded, so clean up.
        dep_map_.erase(path);
        load_time_map_.erase(path);
        return false;
    }

  private:
    //! Maps a path to time it was loaded.
    Map_<Util::Time> load_time_map_;

    //! Maps a path to paths it depends on.
    Map_<std::vector<std::string>> dep_map_;

    //! Returns false if the path has dependency paths and any of them are not
    //! valid.
    bool AreDependenciesValid_(const std::string &path) {
        bool is_valid = true;
        auto it = dep_map_.find(path);
        if (it != dep_map_.end()) {
            for (const std::string &dep_path: it->second) {
                // IsValid() cleans up if not valid, so make sure to call it
                // for all dependencies rather than stopping.
                if (! IsValid(dep_path))
                    is_valid = false;
            }
        }
        return is_valid;
    }
};

// ----------------------------------------------------------------------------
// ResourceManager::Tracker_ class definition.
// ----------------------------------------------------------------------------

//! ResourceManager::Tracker_ is an internal class used to track which
//! resources have been loaded, and when.
class ResourceManager::Tracker_ {
  public:
    NodePtr GetNode(const std::string &path,
                    const LoadFunc<NodePtr> load_func) {
        return GetItem_<NodePtr>(path, "node", node_map_);
    }

    ImagePtr GetTextureImage(const std::string &path,
                             const LoadFunc<ImagePtr> load_func) {
        return GetItem_<ImagePtr>(path, "texture", texture_map_, load_func);
    }

    std::string GetShaderSource(const std::string &path,
                                const LoadFunc<std::string> load_func) {
        return GetItem_<std::string>(path, "shader", shader_map_, load_func);
    }

    void AddDependency(const std::string &owner_path,
                       const std::string &dep_path) {
        dep_tracker_.AddDependency(owner_path, dep_path);
    }

  private:
    //! Maps file path to a Node.
    Map_<NodePtr>           node_map_;
    //! Maps file path to an Image.
    Map_<ImagePtr>          texture_map_;
    //! Maps file path to a shader source string.
    Map_<std::string>       shader_map_;

    //! Stack of paths being loaded; used for dependency tracking.
    std::stack<std::string> path_stack_;

    //! Manages dependencies and modification time checking.
    DependencyTracker_      dep_tracker_;

    //! Accesses the item with the given path from the given map, returning it
    //! if it has been added and is still valid (based on modification time of
    //! it and all resources it depends on).
    template <typename T> T GetItem_(const std::string &path,
                                     const std::string &item_type, Map_<T> &map,
                                     const LoadFunc<T> &load_func) {
        // If there is anything on the stack, add a dependency.
        if (! path_stack_.empty()) {
            const std::string &owner_path = path_stack_.top();
            dep_tracker_.AddDependency(owner_path, path);
            // std::cerr << "XXXX " << owner_path << " depends on "
            //           << path << "\n";
        }
        path_stack_.push(path);

        T item = FindItem_<T>(path, map);
        if (item == T()) {
            // std::cerr << "XXXX Loading " << item_type
            //           << " from '" << path << "'\n";
            item = load_func(path);
            dep_tracker_.AddLoadTime(path);
            map[path] = item;
        }
        path_stack_.pop();
        return item;
    }

    //! Implements the Find function for the given type.
    template <typename T> T FindItem_(const std::string &path, Map_<T> &map) {
        auto it = map.find(path);
        if (it != map.end()) {
            // If the item is still valid and all dependencies are valid,
            // return it.
            if (dep_tracker_.IsValid(path))
                return it->second;
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

void ResourceManager::AddDependency(const std::string &owner_path,
                                    const std::string &dep_path) {
    tracker_->AddDependency(owner_path, dep_path);
}

std::string ResourceManager::GetPath_(const std::string &type_name,
                                      const std::string &path) {
    std::filesystem::path full_path(RESOURCE_DIR);
    full_path /= type_name;
    full_path /= path;
    return full_path.native();
}
