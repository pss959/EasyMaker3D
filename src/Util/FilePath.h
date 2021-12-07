#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "Util/Time.h"

namespace Util {

/// \name File Path Helpers
///@{

/// Represents a path to a file.
class FilePath : private std::filesystem::path {
  public:
    FilePath() {}

    /// Constructor from a string.
    FilePath(const char *path)        : BaseType_(path) {}
    /// Constructor from a string.
    FilePath(const std::string &path) : BaseType_(path) {}

    /// Assignment operator.
    FilePath & operator=(const char *path);
    /// Assignment operator.
    FilePath & operator=(const std::string &path);

    /// Clears to initial (empty) state.
    void Clear() { clear(); }

    /// Converts to a string. Note that this always uses the canonical form
    /// with forward slashes as separators and no drive letters.
    std::string ToString() const;

    /// Converts to a string native to the operating system. This should be
    /// used instead of ToString() when the path is to be used to open a file
    /// for reading or writing.
    std::string ToNativeString() const;

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

    /// Returns the file at the end of the path, if any.
    std::string GetFileName() const;

    /// Returns the extension of the file at the end of the path (including the
    /// dot), if any.
    std::string GetExtension() const;

    /// Returns a FilePath that represents this FilePath when made relative to
    /// the given base FilePath. If this FilePath is absolute, it just returns
    /// it untouched.
    FilePath MakeRelativeTo(const FilePath &base_path) const;

    /// Returns a Util::Time instance representing the last modification time
    /// of the file, which must exist.
    Time GetModTime() const;

    /// If this FilePath represents a directory, this sets subdirs to a sorted
    /// list of names of all subdirectories in it and sets files to a sorted
    /// list of names of all files in it. Otherwise, it just sets them to
    /// empty. If include_hidden is true, this includes hidden files and
    /// subdirectories.
    void GetContents(std::vector<std::string> &subdirs,
                     std::vector<std::string> &files,
                     bool include_hidden);

    /// Removes the file with the path. Used primarily for removing temporary
    /// files for testing.
    void Remove();

    /// Joins two paths, returning the result. The second path must be relative.
    static FilePath Join(const FilePath &p0, const FilePath &p1);

    /// Returns a path to the resource directory, which comes from the
    /// RESOURCE_DIR environment variable.
    static FilePath GetResourceBasePath();

    /// Constructs a path to a resource file. The type of resource is indicated
    /// by the given string, which is used as a subdirectory. The given path is
    /// relative to subdirectory.
    static FilePath GetResourcePath(const std::string &type_name,
                                    const FilePath &sub_path);

    /// If the given path is absolute, this returns it. Otherwise, returns the
    /// result of calling GetResourcePath().
    static FilePath GetFullResourcePath(const std::string &subdir,
                                        const FilePath &path);

    /// Returns a path to the user's home directory, which is OS-dependent.
    static FilePath GetHomeDirPath();

    /// Returns a path to the directory that contains the user's settings
    /// file, which is OS-dependent.
    static FilePath GetSettingsDirPath();

    /// Returns a path to the test data directory, which comes from the
    /// TEST_DATA_DIR environment variable.
    static FilePath GetTestDataPath();

    /// Returns a path to a directory to use for temporary files.
    static FilePath GetTempFilePath();

    /// Returns the separator to use for parts of a path.
    static std::string GetSeparator();

    /// Returns a hash value for use in std::hash.
    std::size_t GetHashValue() const { return hash_value(*this); }

    /// Returns true if the path is not empty.
    operator bool() const { return ! empty(); }

    /// Equality testing.
    bool operator==(const FilePath &p) const {
        return static_cast<const BaseType_>(p) ==
            static_cast<const BaseType_>(*this);
    }
    /// Inequality testing.
    bool operator!=(const FilePath &p) const { return ! (p == *this); }

  private:
    using BaseType_ = std::filesystem::path;
};

}  // namespace Util

// Specialize std::hash() for Util::FilePath.
namespace std {
template <> struct hash<Util::FilePath> {
    std::size_t operator()(const Util::FilePath &path) const {
        return path.GetHashValue();
    }
};
}
