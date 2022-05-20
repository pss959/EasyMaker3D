#pragma once

#include <string>
#include <unordered_map>

#include <ion/gfx/image.h>

#include "Base/Memory.h"
#include "Util/FilePath.h"

namespace SG {

DECL_SHARED_PTR(Tracker);

/// The Tracker class stores associations between (absolute) file paths and
/// data read from those files. It can be used to guarantee that a file is read
/// only once unless the file has been modified since last read.
///
/// \ingroup SG
class Tracker {
  public:
    Tracker();
    ~Tracker();

    /// \name Adding Tracked Resources
    /// Each of these functions adds data of a specific type to the Tracker,
    /// associating it with its (absolute) file path. This sets the load time
    /// for the data to the current time.
    ///@{
    void AddString(const FilePath &path, const std::string &s);
    void AddImage(const FilePath &path,  const ion::gfx::ImagePtr &image);
    ///@}

    /// \name Accessing Tracked Resources

    /// Each of these functions looks for added data of a specific type
    /// associated with a given file path. A null pointer or empty string is
    /// returned if no data was found or if it was but its file has changed
    /// since it was loaded.
    ///@{
    std::string        FindString(const FilePath &path);
    ion::gfx::ImagePtr FindImage(const FilePath &path);
    ///@}

    /// Adds an additional external dependency between the given files so that
    /// the owner will be marked as out of date if the dependency is modified.
    void AddDependency(const FilePath &owner_path, const FilePath &dep_path);

  private:
    /// Convenient alias for a map from a FilePath to an item.
    template <typename T> using PathMap_ = std::unordered_map<FilePath, T>;

    PathMap_<std::string>        string_map_;
    PathMap_<ion::gfx::ImagePtr> image_map_;

    class DependencyTracker_;
    /// Handles dependency tracking.
    std::unique_ptr<DependencyTracker_> dep_tracker_;

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
