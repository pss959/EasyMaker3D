#include "SG/FileMap.h"

#include <functional>
#include <unordered_map>
#include <vector>

#include "SG/Exception.h"
#include "Util/KLog.h"
#include "Util/UTime.h"

namespace SG {

// ----------------------------------------------------------------------------
// FileMap::DependencyMap_ class.
// ----------------------------------------------------------------------------

/// The DependencyMap_ class tracks dependencies between files specified by
/// their full paths. Each file is stored along with a UTime at which it was
/// last accessed. The times are used to determine whether loaded file data is
/// still valid.
class FileMap::DependencyMap_ {
  public:
    /// Adds the load time for the given path using the current time.
    void AddLoadTime(const FilePath &path) {
        load_time_map_[path] = UTime::Now();
    }

    /// Adds a dependency between the given full paths.
    void AddDependency(const FilePath &owner_path, const FilePath &dep_path) {
        auto it = dep_map_.find(owner_path);
        if (it == dep_map_.end())
            dep_map_[owner_path] = std::vector<FilePath>(1, dep_path);
        else
            it->second.push_back(dep_path);
    }

    /// Returns true if data for the given path is still known to be valid,
    /// meaning that its file has not been modified since it was loaded and
    /// that all dependencies are also still valid. If the path is no longer
    /// valid, it is removed from the DependencyMap_.
    bool IsValid(const FilePath &path) {
        // Make sure the file with the path still exists.
        if (path.Exists()) {
            // Get the load time for the path and compare it to the last
            // modification time of the file.
            auto it = load_time_map_.find(path);
            if (it != load_time_map_.end()) {
                if (path.GetModTime() <= it->second &&
                    AreDependenciesValid_(path))
                    return true;
                KLOG('A', "Item for path '" << path.ToString()
                     << "' invalid because of modification");
            }
            else {
                KLOG('A', "Item for path '" << path.ToString()
                     << "' invalid because time never tracked");
            }
        }
        else {
            KLOG('A', "Item for path '" << path.ToString()
                 << "' invalid because no longer exists");
        }

        // No longer valid. It will be reloaded, so clean up.
        dep_map_.erase(path);
        load_time_map_.erase(path);
        return false;
    }

  private:
    /// Maps a path to time it was loaded.
    PathMap_<UTime>                 load_time_map_;

    /// Maps a path to paths it depends on.
    PathMap_<std::vector<FilePath>> dep_map_;

    /// Returns false if the path has dependency paths and any of them are not
    /// valid.
    bool AreDependenciesValid_(const FilePath &path) {
        bool is_valid = true;
        auto it = dep_map_.find(path);
        if (it != dep_map_.end()) {
            for (const FilePath &dep_path: it->second) {
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
// FileMap implementation.
// ----------------------------------------------------------------------------

FileMap::FileMap() : dep_map_(new DependencyMap_) {
}

FileMap::~FileMap() {
}

void FileMap::AddString(const FilePath &path, const Str &s) {
    KLOG('A', "Adding string " << &s << " with size " << s.size()
         << " for path '" << path.ToString() << "'");
    string_map_[path] = s;
    dep_map_->AddLoadTime(path);
}

void FileMap::AddImage(const FilePath &path, const ion::gfx::ImagePtr &image) {
    KLOG('A', "Adding Image " << image.Get() << " for path '"
         << path.ToString() << "'");
    image_map_[path] = image;
    dep_map_->AddLoadTime(path);
}

Str FileMap::FindString(const FilePath &path) {
    return FindItem_<Str>(path, string_map_);
}

ion::gfx::ImagePtr FileMap::FindImage(const FilePath &path) {
    return FindItem_<ion::gfx::ImagePtr>(path, image_map_);
}

void FileMap::AddDependency(const FilePath &owner_path,
                            const FilePath &dep_path) {
    KLOG('A', "Adding dependency of '" << owner_path.ToString()
         << "' on '" << dep_path.ToString() << "'");
    dep_map_->AddDependency(owner_path, dep_path);
}

bool FileMap::IsPathStillValid_(const FilePath &path) {
    return dep_map_->IsValid(path);
}

}  // namespace SG
