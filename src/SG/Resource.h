#pragma once

#include "SG/Object.h"
#include "Util/FilePath.h"

namespace SG {

//! SG::Resource is an abstract base class for SG classes that are read from
//! files. It stores the file path the resource was read from.
class Resource : public Object {
  public:
    //! Returns the path that the resource object was read from.
    const Util::FilePath & GetFilePath() const { return file_path_; }

  protected:
    //! The constructor is passed the FilePath the resource was read from.
    Resource(const Util::FilePath &path) : file_path_(path) {}

  private:
    Util::FilePath file_path_;
};

}  // namespace SG
