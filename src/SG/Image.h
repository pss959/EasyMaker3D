#pragma once

#include <string>

#include <ion/gfx/image.h>

#include "Parser/ObjectSpec.h"
#include "SG/Object.h"
#include "Util/FilePath.h"

namespace SG {

//! An Image object wraps an Ion image read from a file.
class Image : public Object {
  public:
    //! Returns the path that the image was read from.
    Util::FilePath GetFilePath() const { return path_; }

     //! Returns the Ion image.
    const ion::gfx::ImagePtr & GetIonImage() const { return ion_image_; }

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    std::string path_;
    //!@}

    ion::gfx::ImagePtr ion_image_;  //! Associated Ion Image.
};

}  // namespace SG
