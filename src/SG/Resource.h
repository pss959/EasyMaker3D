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

  private:
    // Parsed fields.
    std::string path_;
};

}  // namespace SG
