#pragma once

#include <ion/gfx/image.h>

#include "Graph/Resource.h"

namespace Graph {

//! An Image object wraps an Ion image. It is derived from Resource so that it
//! maintains the path to the file the image was read from.
class Image : public Resource {
  public:
    //! Returns the Ion image.
    const ion::gfx::ImagePtr & GetIonImage() const { return i_image_; }

  private:
    ion::gfx::ImagePtr  i_image_;  //! Associated Ion Image.

    friend class Reader_;
};

}  // namespace Graph
