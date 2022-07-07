#pragma once

#include <functional>

#include "Base/Memory.h"
#include "SG/Image.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(ProceduralImage);

/// ProceduralImage is a derived Image object that generates the Image data
/// using a function. If no function is supplied, this creates a 1x1 white
/// image.
///
/// \ingroup SG
class ProceduralImage : public Image {
  public:
    /// Typedef for function used to generate an Ion Image.
    typedef std::function<ion::gfx::ImagePtr()> ImageFunc;

    /// Sets the procedural function to invoke to generate the image.
    void SetFunction(const ImageFunc &func) { func_ = func; }

    /// Implements this to generate a procedural image.
    virtual ion::gfx::ImagePtr CreateIonImage(FileMap &file_map) override;

    /// This can be called to regenerate the image if something it depends on
    /// has changed.
    void RegenerateImage();

  protected:
    ProceduralImage() {}

  private:
    /// The function to invoke.
    ImageFunc func_;

    /// Actually generates the image.
    ion::gfx::ImagePtr GenerateImage_();

    friend class Parser::Registry;
};

}  // namespace SG
