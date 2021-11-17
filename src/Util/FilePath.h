#pragma once

#include <string>

#include "Util/Time.h"

namespace Util {

/// \name File Path Helpers
///@{

/// Represents a path to a file.
class FilePath : public std::filesystem::path {
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

    /// Converts to a string.
    std::string ToString() const;

    /// Returns true if the file specified by the path exists.
    bool Exists() const;

    /// Returns true if the path represents a directory.
    bool IsDirectory() const;

    /// Returns true if the file is an absolute path.
    bool IsAbsolute() const;

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

  private:
    using BaseType_ = std::filesystem::path;
};

}  // namespace Util

// Specialize std::hash() for Util::FilePath.
namespace std {
template <> struct hash<Util::FilePath> {
    std::size_t operator()(const Util::FilePath &path) const {
        return hash_value(path);
    }
};
}
