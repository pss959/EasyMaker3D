#pragma once

#include <string>

#include <ion/gfx/image.h>

#include "NParser/ObjectSpec.h"
#include "SG/Resource.h"

namespace SG {

//! An Image object wraps an Ion image. It is derived from Resource so that it
//! reads and maintains the path to the file the image was read from.
class Image : public Resource {
  public:
    //! Returns the Ion image.
    const ion::gfx::ImagePtr & GetIonImage() const { return ion_image_; }

    virtual void SetUpIon(IonContext &context) override;

    static NParser::ObjectSpec GetObjectSpec();

  private:
    ion::gfx::ImagePtr ion_image_;  //! Associated Ion Image.
};

}  // namespace SG
