#pragma once

#include "Graph/Object.h"
#include "Util/FilePath.h"

namespace Graph {

class Reader_;

//! Resource is an abstract base class for Graph classes that are read from
//! files. It stores the file path the resource was read from.
class Resource : public Object {
  public:
    //! Returns the path that the resource object was read from.
    const Util::FilePath & GetFilePath() const { return file_path_; }

  protected:
    Resource() {}

    //! Sets the file path.
    virtual void SetFilePath_(const Util::FilePath &path) { file_path_ = path; }

  private:
    Util::FilePath file_path_;

    friend class Reader_;
};

}  // namespace Graph
