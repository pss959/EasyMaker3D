#pragma once

#include <filesystem>
#include <string>

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
  private:
    using BaseType_ = std::filesystem::path;
};

}  // namespace Util
