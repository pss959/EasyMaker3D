#include "SG/Image.h"

#include <assert.h>

#include "NParser/Exception.h"
#include "SG/SpecBuilder.h"
#include "Util/Read.h"

namespace SG {

void Image::Finalize() {
    assert(! ion_image_);
    ion_image_ = Util::ReadImage(path_);
    if (! ion_image_)
        throw NParser::Exception(path_, "Unable to open or read image file");
}

std::vector<NParser::FieldSpec> Image::GetFieldSpecs_() {
    SG::SpecBuilder<Image> builder;
    builder.AddString("path", &Image::path_);
    return builder.GetSpecs();
}

}  // namespace SG
