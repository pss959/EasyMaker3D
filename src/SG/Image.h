#pragma once

#include <string>

#include <ion/gfx/image.h>

#include "NParser/FieldSpec.h"
#include "SG/Resource.h"

namespace SG {

//! An Image object wraps an Ion image. It is derived from Resource so that it
//! maintains the path to the file the image was read from.
class Image : public Resource {
  public:
    //! The constructor is passed the FilePath the image was read from.
    Image(const Util::FilePath &path) : Resource(path) {}

    //! Returns the Ion image.
    const ion::gfx::ImagePtr & GetIonImage() const { return ion_image_; }

  private:
    ion::gfx::ImagePtr  ion_image_;  //! Associated Ion Image.

    std::string path_;

    //! Redefines this to read and set up the Ion Image.
    virtual void Finalize() override;

    static std::vector<NParser::FieldSpec> GetFieldSpecs_();
};

}  // namespace SG
