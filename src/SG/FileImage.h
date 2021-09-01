#pragma once

#include <string>

#include <ion/gfx/image.h>

#include "Parser/ObjectSpec.h"
#include "SG/Image.h"
#include "Util/FilePath.h"

namespace SG {

//! A FileImage object wraps an Ion image read from a file.
class FileImage : public Image {
  public:
    //! Returns the path that the image was read from.
    Util::FilePath GetFilePath() const { return path_; }

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    std::string path_;
    //!@}
};

}  // namespace SG
