#include "SG/Image.h"

#include <assert.h>

#include "NParser/Exception.h"
#include "SG/SpecBuilder.h"
#include "Util/Read.h"

namespace SG {

NParser::ObjectSpec Image::GetObjectSpec() {
    return NParser::ObjectSpec{
        "Image", []{ return new Image; },
        Resource::GetObjectSpec().field_specs };
}

void Image::Finalize() {
    assert(! ion_image_);
    ion_image_ = Util::ReadImage(GetFilePath());
    if (! ion_image_)
        throw NParser::Exception(GetFilePath(),
                                 "Unable to open or read image file");
}

}  // namespace SG
