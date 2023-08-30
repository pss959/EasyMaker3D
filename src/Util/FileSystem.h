#pragma once

#include <filesystem>

#include "Util/UTime.h"

/// Wrappers around file system utilities. Most of these functions are virtual
/// to allow derived classes to redefine them for testing.
///
/// \ingroup Utility
class FileSystem {
  public:
    using Path = std::filesystem::path;

    /// Converts a path to a string native to the operating system. This should
    /// be used when a path is to be used to open a file for reading or
    /// writing.
    virtual Str ToNativeString(const Path &path) const;

    /// Returns true if the file specified by the path exists.
    virtual bool Exists(const Path &path) const;

    /// Returns true if the path represents a directory.
    virtual bool IsDirectory(const Path &path) const;

    /// Returns true if the path is absolute.
    virtual bool IsAbsolute(const Path &path) const;

    /// Returns true if the path represents a hidden file or directory, doing
    /// the best it can on all platforms.
    virtual bool IsHidden(const Path &path) const;

    /// Returns a UTime instance representing the last modification time of the
    /// file specified by \p path, which must exist.
    virtual UTime GetModTime(const Path &path) const;

    /// Returns the contents of the directory represented by \p path as sorted
    /// lists of subdirectory names and file names. Asserts if path is not a
    /// directory. If \p include_hidden is true, this includes hidden files and
    /// subdirectories.
    virtual void GetDirectoryContents(const Path &path, StrVec &subdirs,
                                      StrVec &files, bool include_hidden) const;

    /// Removes the file with the path. Used primarily for removing temporary
    /// files for testing.
    virtual void Remove(const Path &path) const;

    /// Creates all directories that do not already exist in the path. This
    /// should be called only on a directory path.  Returns false if anything
    /// fails.
    virtual bool CreateDirectories(const Path &path) const;

    /// Changes the current directory to this one. Use with caution.
    virtual void MakeCurrent(const Path &path) const;

    /// Returns the current directory.
    virtual Path GetCurrent() const;

    /// Returns a path to the user's home directory, which is OS-dependent.
    virtual Path GetHomeDirPath() const;

    /// Returns a path to the settings directory, which is OS-dependent. The
    /// name of the application is supplied.
    virtual Path GetSettingsDirPath(const Str &app_name) const;

    /// Returns a path to a directory to use for temporary files.
    virtual Path GetTempFilePath() const;

    /// Returns the separator to use for parts of a path.
    virtual Str GetSeparator() const;

  private:
    /// OS-dependent access to environment variables.
    static Str GetEnvVar_(const Str &name);
};
