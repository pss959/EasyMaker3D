#pragma once

#include <ion/gfxutils/shapeutils.h>

#include "Parser/ObjectSpec.h"
#include "SG/TriMeshShape.h"
#include "Util/FilePath.h"

namespace SG {

//! ImportedShape is a derived TriMeshShape that loads its Ion shape from a
//! file. These files are not tracked by the Tracker to save some memory.
class ImportedShape : public TriMeshShape {
  public:
    //! Returns the path that the shape was read from.
    Util::FilePath GetFilePath() const { return path_; }

    virtual ion::gfx::ShapePtr CreateIonShape() override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    std::string path_;
    //!@}
};

}  // namespace SG
