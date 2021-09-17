#pragma once

#include <filesystem>
#include <string>

#include "Assert.h"
#include "Util/Time.h"

namespace Util {

//! \name File Path Helpers
//!@{

//! Represents a path to a file.
class FilePath : public std::filesystem::path {
  public:
    FilePath() {}

    //! Constructor from a string.
    FilePath(const char *path)        : BaseType_(path) {}
    //! Constructor from a string.
    FilePath(const std::string &path) : BaseType_(path) {}

    //! Assignment operator.
    FilePath & operator=(const char *path) {
        BaseType_::operator=(path);
        return *this;
    }
    //! Assignment operator.
    FilePath & operator=(const std::string &path) {
        BaseType_::operator=(path);
        return *this;
    }

    //! Converts to a string.
    std::string ToString() const {
        return this->native();
    }

    //! Returns true if the file specified by the path exists.
    bool Exists() const {
        return std::filesystem::exists(*this);
    }

    //! Returns true if the file is an absolute path.
    bool IsAbsolute() const {
        return is_absolute();
    }

    //! Returns the extension of the file at the end of the path (including the
    //! dot), if any.
    std::string GetExtension() const {
        return this->extension();
    }

    //! Returns a FilePath that represents this FilePath when made relative to
    //! the given base FilePath. If this FilePath is absolute, it just returns
    //! it untouched.
    FilePath MakeRelativeTo(const FilePath &base_path) const {
        if (IsAbsolute())
            return *this;
        // If the base_path exists and is not known to be a directory, remove
        // the last component.
        else if (std::filesystem::exists(base_path) &&
                 ! std::filesystem::is_directory(base_path))
            return FilePath(base_path.parent_path() / *this);
        else
            return FilePath(base_path / *this);
    }

    //! Returns a Util::Time instance representing the last modification time
    //! of the file, which must exist.
    Time GetModTime() const {
        ASSERT(Exists());
        return Time(std::filesystem::last_write_time(*this));
    }

    //! Returns a path to the resource directory, which comes from the
    //! RESOURCE_DIR environment variable.
    static FilePath GetResourceBasePath() {
        return FilePath(RESOURCE_DIR);
    }

    //! Constructs a path to a resource file. The type of resource is indicated
    //! by the given string, which is used as a subdirectory. The given path is
    //! relative to subdirectory.
    static FilePath GetResourcePath(const std::string &type_name,
                                    const FilePath &sub_path) {
        FilePath path = GetResourceBasePath();
        path /= type_name;
        path /= sub_path;
        return path;
    }

    //! If the given path is absolute, this returns it. Otherwise, returns the
    //! result of calling GetResourcePath().
    static FilePath GetFullResourcePath(const std::string &subdir,
                                        const FilePath &path) {
        if (path.IsAbsolute())
            return path;
        else
            return GetResourcePath(subdir, path);
    }

    //! Returns a path to the test data directory, which comes from the
    //! TEST_DATA_DIR environment variable.
    static FilePath GetTestDataPath() {
        return FilePath(TEST_DATA_DIR);
    }

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
