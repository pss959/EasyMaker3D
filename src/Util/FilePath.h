#pragma once

#include <assert.h>

#include <filesystem>
#include <string>

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

    //! Returns a Util::Time instance representing the last modification time
    //! of the file, which must exist.
    Time GetModTime() const {
        assert(Exists());
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
