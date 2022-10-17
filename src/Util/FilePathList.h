#pragma once

#include <vector>

#include "Util/FilePath.h"

/// A FilePathList stores a collection of FilePath instances and allows for
/// switching between them and other operations. It is used by the FilePanel
/// for managing paths.
///
/// \ingroup Utility
class FilePathList {
  public:
    virtual ~FilePathList() {}

    /// Directions for switching paths.
    enum class Direction { kUp, kForward, kBack, kHome };

    /// Sets up with an initial path to use. The path must be absolute.
    void Init(const FilePath &initial_path);

    /// Returns the current path.
    const FilePath & GetCurrent() const { return paths_[cur_index_]; }

    /// Returns true if it is possible to go in the given direction from the
    /// current path.
    bool CanGoInDirection(Direction dir) const;

    /// Goes to the new path in the given direction. Returns the new path.
    const FilePath & GoInDirection(Direction dir);

    /// Adds a new path. If the path is not absolute, it is first made absolute
    /// by joining it to the current path. Returns the absolute path.
    const FilePath & AddPath(const FilePath &path);

    /// Uses FilePath::GetContents() on the current path if it is a directory
    /// or its parent directory otherwise.
    virtual void GetContents(std::vector<std::string> &subdirs,
                             std::vector<std::string> &files,
                             const std::string &extension,
                             bool include_hidden) const;

    /// Dumps current state for help with debugging.
    void Dump();

  private:
    std::vector<FilePath> paths_;
    size_t                cur_index_ = 0;

    /// If the given path is not absolute, this makes it absolute using the
    /// current path.
    FilePath MakeAbsolute_(const FilePath &path);
};
