#include "SG/ProceduralImage.h"

#include "SG/Exception.h"

namespace SG {

ion::gfx::ImagePtr ProceduralImage::CreateIonImage(FileMap &file_map) {
    return GenerateImage_();
}

void ProceduralImage::RegenerateImage() {
    if (GetIonImage())
        ReplaceImage(GenerateImage_());
}

ion::gfx::ImagePtr ProceduralImage::GenerateImage_() {
    if (func_) {
        return func_();
    }
    else {
        uint8 pix[4]{ 0xff, 0xff, 0xff, 0xff };
        ion::gfx::ImagePtr image(new ion::gfx::Image);
        image->Set(ion::gfx::Image::kRgb888, 1, 1,
                   ion::base::DataContainer::CreateAndCopy(
                       pix, 4, true, ion::base::AllocatorPtr()));
        return image;
    }
}

}  // namespace SG
