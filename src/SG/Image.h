#pragma once

#include <ion/gfx/image.h>

#include "SG/Object.h"

namespace SG {

class Tracker;

//! Image is an abstract base class for objects that wrap an Ion image.
class Image : public Object {
  public:
     //! Returns the Ion image, creating it first if necessary. A Tracker is
     //! supplied to look up file-based images.
    const ion::gfx::ImagePtr & GetIonImage(Tracker &tracker) {
        if (! ion_image_)
            ion_image_ = CreateIonImage(tracker);
        return ion_image_;
    }

  protected:
    //! Protected constructor to make this abstract.
    Image() {}

    //! Derived classes must implement this to create an Ion Image.
    virtual ion::gfx::ImagePtr CreateIonImage(Tracker &tracker) = 0;

  private:
    ion::gfx::ImagePtr ion_image_;  //! Associated Ion Image.
};

}  // namespace SG
