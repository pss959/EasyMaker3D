#pragma once

#include <string>

#include <ion/gfx/image.h>

#include "NParser/FieldSpec.h"
#include "SG/Resource.h"

namespace SG {

//! An Image object wraps an Ion image. It is derived from Resource so that it
//! reads and maintains the path to the file the image was read from.
class Image : public Resource {
  public:
    //! Returns the Ion image.
    const ion::gfx::ImagePtr & GetIonImage() const { return ion_image_; }

  private:
    ion::gfx::ImagePtr ion_image_;  //! Associated Ion Image.

    //! Redefines this to read and set up the Ion Image.
    virtual void Finalize() override;
};

}  // namespace SG
