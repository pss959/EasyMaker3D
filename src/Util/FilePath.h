//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include "Util/UTime.h"

class FileSystem;

/// Represents a path to a file.
///
/// \ingroup Utility
class FilePath : private std::filesystem::path {
  public:
    FilePath() {}

    /// Constructor from a string.
    FilePath(const char *path) : FSPath_(path) {}
    /// Constructor from a string.
    FilePath(const Str &path) : FSPath_(path) {}

    /// Assignment operator.
    FilePath & operator=(const char *path);
    /// Assignment operator.
    FilePath & operator=(const Str &path);

    /// Clears to initial (empty) state.
    void Clear() { clear(); }

    /// Converts to a string. Note that this always uses the canonical form
    /// with forward slashes as separators and no drive letters.
    Str ToString() const;

    /// Converts to a string native to the operating system. This should be
    /// used instead of ToString() when the path is to be used to open a file
    /// for reading or writing.
    Str ToNativeString() const;

    /// Returns true if the file specified by the path exists.
    bool Exists() const;

    /// Returns true if the path represents a directory.
    bool IsDirectory() const;

    /// Returns true if the file is an absolute path.
    bool IsAbsolute() const;

    /// Returns true if the path represents a hidden file or directory, doing
    /// the best it can on all platforms.
    bool IsHidden() const;

    /// Returns a path to the parent directory, or an empty path if there is
    /// none.
    FilePath GetParentDirectory() const;

    /// Returns the file at the end of the path, if any. If remove_extension is
    /// true, this returns just the part before the extension, if any.
    Str GetFileName(bool remove_extension = false) const;

    /// Returns the extension of the file at the end of the path (including the
    /// dot), if any.
    Str GetExtension() const;

    /// Adds the given extension (which should start with a dot) to the
    /// path. If the extension is already there, this does nothing.
    void AddExtension(const Str &extension);

    /// Replaces the current extension in the path with the given one (which
    /// should start with a dot). If there was no extension, this just adds the
    /// new extension.
    void ReplaceExtension(const Str &extension);

    /// If this FilePath is relative, this appends it after the given base path
    /// and returns the result. If the base path is a file, this uses its
    /// parent path. If this FilePath is absolute, it is returned untouched.
    FilePath AppendRelative(const FilePath &base_path) const;

    /// Returns a FilePath that represents this FilePath when made relative to
    /// the given base FilePath.
    FilePath MakeRelativeTo(const FilePath &base_path) const;

    /// If the given FilePath is absolute, this returns it. Otherwise, it
    /// returns an absolute path, assuming the given path is relative to the
    /// current directory.
    FilePath GetAbsolute() const;

    /// Returns a UTime instance representing the last modification time of the
    /// file, which must exist.
    UTime GetModTime() const;

    /// If this FilePath represents a directory, this sets subdirs to a sorted
    /// list of names of all subdirectories in it and sets files to a sorted
    /// list of names of all files in it. Otherwise, it just sets them to
    /// empty. If extension is empty, only files with a matching extension are
    /// added. If include_hidden is true, this includes hidden files and
    /// subdirectories.
    void GetContents(StrVec &subdirs, StrVec &files, const Str &extension,
                     bool include_hidden) const;

    /// Removes the file with the path. Used primarily for removing temporary
    /// files for testing.
    void Remove() const;

    /// Creates all directories that do not already exist in this path. This
    /// should be called only on a directory path.  Returns false if anything
    /// fails.
    bool CreateDirectories() const;

    /// Changes the current directory to this one. Use with caution.
    void MakeCurrent() const;

    /// Returns the current directory.
    static FilePath GetCurrent();

    /// Joins two paths, returning the result. The second path must be relative.
    static FilePath Join(const FilePath &p0, const FilePath &p1);

    /// Returns a path to the resource directory, which comes from the
    /// RESOURCE_DIR environment variable.
    static FilePath GetResourceBasePath();

    /// Constructs a path to a resource directory or file. The type of resource
    /// is indicated by the type_name string, which is used as a
    /// subdirectory. If the sub_path is empty, this returns the resource
    /// directory. Otherwise, it creates a path to the sub_path relative to the
    /// subdirectory.
    static FilePath GetResourcePath(const Str &type_name,
                                    const FilePath &sub_path);

    /// If the given path is absolute, this returns it. Otherwise, returns the
    /// result of calling GetResourcePath().
    static FilePath GetFullResourcePath(const Str &subdir,
                                        const FilePath &path);

    /// Returns a path to the user's home directory, which is OS-dependent.
    static FilePath GetHomeDirPath();

    /// Returns a path to the settings directory, which is OS-dependent. The
    /// name of the application is supplied.
    static FilePath GetSettingsDirPath(const Str &app_name);

    /// Returns a path to the test data directory, which comes from the
    /// TEST_DATA_DIR environment variable.
    static FilePath GetTestDataPath();

    /// Returns a path to a directory to use for temporary files.
    static FilePath GetTempFilePath();

    /// Returns the separator to use for parts of a path.
    static Str GetSeparator();

    /// Returns a hash value for use in std::hash.
    std::size_t GetHashValue() const { return hash_value(*this); }

    /// Returns true if the path is not empty.
    operator bool() const { return ! empty(); }

    /// Equality testing.
    bool operator==(const FilePath &p) const = default;

  private:
    using FSPath_ = std::filesystem::path;

    /// Shortcut to get the current FileSystem instance.
    static FileSystem & GetFS_();

    /// Creates a FilePath from an std::filesystem::path.
    static FilePath FromFSPath_(const FSPath_ &fs_path);
};

// Specialize std::hash() for FilePath.
namespace std {
template <> struct hash<FilePath> {
    std::size_t operator()(const FilePath &path) const {
        return path.GetHashValue();
    }
};
}
