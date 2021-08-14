#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Graph/Typedefs.h"
#include "Util/FilePath.h"

namespace Input {

//! The Tracker class stores associations between paths and objects derived
//! from Graph::Resource representing resources read from files. It can be used
//! to guarantee that a file is read only once unless the file has been
//! modified since last read. All specified file paths are relative to
//! RESOURCE_DIR.
//
//! \ingroup Input
class Tracker {
  public:
    //! Convenience typedef for a path.
    typedef Util::FilePath Path;

    Tracker();
    ~Tracker();

    //! \name Adding Tracked Resources
    //! Each of these functions adds a derived Graph::Resource of a specific
    //! type to the Tracker, associating it with its file path. This sets the
    //! load time for the resource to the current time.
    //!@{
    void AddScene(const Graph::ScenePtr &scene);
    void AddImage(const Graph::ImagePtr &image);
    void AddShaderSource(const Graph::ShaderSourcePtr &source);
    //!@}

    //! \name Finding Resources
    //! Each of these functions looks for an added Graph::Resource of a
    //! specific type associated with a given file path. A null pointer is
    //! returned if no resource is found or if one is found but its file has
    //! changed since it was loaded.
    //!@{
    Graph::ScenePtr        FindScene(const Path &path);
    Graph::ImagePtr        FindImage(const Path &path);
    Graph::ShaderSourcePtr FindShaderSource(const Path &path);
    //!@}

    //! Adds an additional external dependency between the given files so that
    //! the owner will be marked as out of date if the dependency is modified.
    void AddDependency(const Path &owner_path, const Path &dep_path);

  private:
    //! Convenient alias for a map from a Path to an item.
    template <typename T> using PathMap_ = std::unordered_map<Path, T>;

    PathMap_<Graph::ScenePtr>        scene_map_;
    PathMap_<Graph::ImagePtr>        image_map_;
    PathMap_<Graph::ShaderSourcePtr> shader_map_;

    class DependencyTracker_;
    //! Handles dependency tracking.
    std::unique_ptr<DependencyTracker_> dep_tracker_;

    //! XXXX
    template <typename T> T FindItem_(const Path &path, PathMap_<T> &map) {
        auto it = map.find(path);
        if (it != map.end()) {
            if (IsPathStillValid_(path))
                return it->second;
            else
                map.erase(it);
        }
        return T();  // Not found.
    }

    // XXXX
    bool IsPathStillValid_(const Path &path);
};

}  // namespace Input
