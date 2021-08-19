#pragma once

#include "SG/Object.h"
#include "Util/FilePath.h"

namespace SG {

//! SG::Resource is an abstract base class for SG classes that are read from
//! files. It stores the file path the resource was read from.
class Resource : public Object {
  public:
    //! Returns the path that the resource object was read from.
    Util::FilePath GetFilePath() const { return path_; }

    static NParser::ObjectSpec GetObjectSpec();

  protected:
    //! Converts the file path (if necessary) to an absolute path, assuming it
    //! is relative to the named subdirectory of the resource directory.
    Util::FilePath GetFullPath(const std::string &subdir) const;

  private:
    // Parsed fields.
    std::string path_;
};

}  // namespace SG
