#pragma once

#include <ion/gfx/image.h>

#include "SG/Object.h"

namespace SG {

//! Image is an abstract base class for objects that wrap an Ion image.
class Image : public Object {
  public:
     //! Returns the Ion image.
    const ion::gfx::ImagePtr & GetIonImage() const { return ion_image_; }

  protected:
    //! Protected constructor to make this abstract.
    Image() {}

    //! Allows derived classes to set the Ion Image.
    void SetIonImage(const ion::gfx::ImagePtr &image) { ion_image_ = image; }

  private:
    ion::gfx::ImagePtr ion_image_;  //! Associated Ion Image.
};

}  // namespace SG
