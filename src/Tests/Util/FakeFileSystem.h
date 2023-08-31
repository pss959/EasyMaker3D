#pragma once

#include <map>

#include "Util/FileSystem.h"
#include "Util/UTime.h"

/// FakeFileSystem is a derived FileSystem class that allows for testing
/// filesystem-related functions without relying on specific files and
/// directories to actually exist. It maintains a map of paths for files and
/// directories that are known to exist.
///
/// Note that this does NOT override GetTempFilePath(), which has to refer to a
/// real file for many tests.
///
/// \ingroup Tests
class FakeFileSystem : public FileSystem {
  public:
    /// Redefines this to return true;
    virtual bool IsFake() const override { return true; }

    /// Adds a file to the map.
    void AddFile(const Str &path_str, bool is_hidden = false) const;

    /// Adds a directory path to the map.
    void AddDir(const Str &path_str, bool is_hidden = false) const;

    /// Sets the last-modified time for a path already in the map.
    void SetModTime(const Str &path_str, const UTime &time) const;

    /// Redefines this to just return the path as is.
    virtual Str ToNativeString(const Path &path) const override;

    /// Returns true if the file is in the map.
    virtual bool Exists(const Path &path) const override;

    /// Returns true if the path is in the map and the kDirectory flag is set.
    virtual bool IsDirectory(const Path &path) const override;

    /// Returns true if the path starts with "/".
    virtual bool IsAbsolute(const Path &path) const override;

    /// Returns true if the path is in the map and the kHidden flag is set.
    virtual bool IsHidden(const Path &path) const override;

    /// Returns the UTime instance passed to SetModTime().
    virtual UTime GetModTime(const Path &path) const override;

    /// Returns contents from the map.
    virtual void GetDirectoryContents(const Path &path, StrVec &subdirs,
                                      StrVec &files,
                                      bool include_hidden) const override;

    /// Removes the file from the map.
    virtual void Remove(const Path &path) const override;

    /// Adds any missing directories to the map.
    virtual bool CreateDirectories(const Path &path) const override;

    /// Stores the path as the current directory.
    virtual void MakeCurrent(const Path &path) const override;

    /// Returns the current directory. This is "/" by default.
    virtual Path GetCurrent() const override;

    /// Returns "/home/user".
    virtual Path GetHomeDirPath() const override;

    /// Returns "/settings/<app_name>".
    virtual Path GetSettingsDirPath(const Str &app_name) const override;

    /// Returns "/".
    virtual Str GetSeparator() const override;

  private:
    /// Struct containing file/directory info for a path.
    struct PathInfo_ {
        bool  is_dir    = false;
        bool  is_hidden = false;
        UTime mod_time;
        PathInfo_() {}
        PathInfo_(bool d, bool h) : is_dir(d), is_hidden(h) {}
    };

    /// This map stores all known paths.
    mutable std::map<Str, PathInfo_> file_map_;

    /// Current directory path.
    mutable Path current_ = Path("/");
};
