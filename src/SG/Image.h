#pragma once

#include <ion/gfx/image.h>

#include "SG/IonContext.h"
#include "SG/Object.h"

namespace SG {

class Tracker;

/// Image is an abstract base class for objects that wrap an Ion image.
class Image : public Object {
  public:
    /// Creates, stores, and returns the Ion Image.
    ion::gfx::ImagePtr SetUpIon(const IonContextPtr &ion_context) {
        if (! ion_image_)
            ion_image_ = CreateIonImage(ion_context->GetTracker());
        return ion_image_;
    }

    /// Returns the Ion image. This is null until SetUpIon() is called.
    const ion::gfx::ImagePtr & GetIonImage() { return ion_image_; }

  protected:
    /// Protected constructor to make this abstract.
    Image() {}

    /// Derived classes must implement this to create an Ion Image.
    virtual ion::gfx::ImagePtr CreateIonImage(Tracker &tracker) = 0;

  private:
    ion::gfx::ImagePtr ion_image_;  /// Associated Ion Image.
};

}  // namespace SG
