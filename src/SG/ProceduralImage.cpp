#include "SG/ProceduralImage.h"

#include "SG/Exception.h"

namespace SG {

ion::gfx::ImagePtr ProceduralImage::CreateIonImage(Tracker &tracker) {
    return GenerateImage_();
}

void ProceduralImage::RegenerateImage() {
    if (GetIonImage())
        ReplaceImage(GenerateImage_());
}

ion::gfx::ImagePtr ProceduralImage::GenerateImage_() {
    if (! func_)
        throw Exception("No function supplied for " + GetDesc());
    return func_();
}

}  // namespace SG
