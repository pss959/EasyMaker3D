#pragma once

#include <unordered_map>

#include <ion/gfx/image.h>

#include "Util/FilePath.h"
#include "Util/Memory.h"

namespace SG {

DECL_SHARED_PTR(FileMap);

/// The FileMap class stores associations between file paths and data read from
/// those files. It can be used to guarantee that a file is read only once
/// unless the file has been modified since last read. Note that all file paths
/// must be absolute except for a release build.
///
/// \ingroup SG
class FileMap {
  public:
    FileMap();
    ~FileMap();

    /// \name Adding Tracked Resources
    /// Each of these functions adds data of a specific type to the FileMap,
    /// associating it with its file path. This sets the load time for the data
    /// to the current time.
    ///@{
    void AddString(const FilePath &path, const Str &s);
    void AddImage(const FilePath &path,  const ion::gfx::ImagePtr &image);
    ///@}

    /// \name Accessing Tracked Resources

    /// Each of these functions looks for added data of a specific type
    /// associated with a given file path. A null pointer or empty string is
    /// returned if no data was found or if it was but its file has changed
    /// since it was loaded.
    ///@{
    Str                FindString(const FilePath &path);
    ion::gfx::ImagePtr FindImage(const FilePath &path);
    ///@}

    /// Adds an additional external dependency between the given files so that
    /// the owner will be marked as out of date if the dependency is modified.
    void AddDependency(const FilePath &owner_path, const FilePath &dep_path);

  private:
    /// Convenient alias for a map from a FilePath to an item.
    template <typename T> using PathMap_ = std::unordered_map<FilePath, T>;

    PathMap_<Str>                string_map_;
    PathMap_<ion::gfx::ImagePtr> image_map_;

    class DependencyMap_;
    /// Handles dependency tracking.
    std::unique_ptr<DependencyMap_> dep_map_;

    /// Looks for an item of the templated type associated with the given
    /// path in the given PathMap_. If it exists and is still valid, this
    /// returns it.
    template <typename T> T FindItem_(const FilePath &path, PathMap_<T> &map) {
        auto it = map.find(path);
        if (it != map.end()) {
            if (IsPathStillValid_(path))
                return it->second;
            else
                map.erase(it);
        }
        return T();  // Not found.
    }

    /// Returns true if the given path is still valid with respect to all
    /// dependencies and load times.
    bool IsPathStillValid_(const FilePath &path);
};

}  // namespace SG
