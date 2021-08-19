#include "SG/Image.h"

#include <assert.h>

#include "NParser/Exception.h"
#include "SG/SpecBuilder.h"
#include "Util/Read.h"

namespace SG {

void Image::SetUpIon(IonContext &context) {
    // XXXX Deal with Tracker!
    if (! ion_image_) {
        ion_image_ = Util::ReadImage(GetFilePath());
        if (! ion_image_)
            throw NParser::Exception(GetFilePath(),
                                     "Unable to open or read image file");
    }
}

NParser::ObjectSpec Image::GetObjectSpec() {
    return NParser::ObjectSpec{
        "Image", false, []{ return new Image; },
        Resource::GetObjectSpec().field_specs };
}

}  // namespace SG
