#include "SG/Image.h"

#include "SG/Exception.h"
#include "SG/SpecBuilder.h"
#include "SG/Tracker.h"
#include "Util/Read.h"

namespace SG {

void Image::SetUpIon(IonContext &context) {
    if (! ion_image_) {
        const Util::FilePath path =
            Util::FilePath::GetFullResourcePath("images", path_);

        // Check the Tracker first to see if the Image was already loaded.
        ion_image_ = context.tracker.FindImage(path);

        if (! ion_image_) {
            ion_image_ = Util::ReadImage(path);
            if (! ion_image_)
                throw Exception("Unable to open or read image file '" +
                                path.ToString() + "'");
            context.tracker.AddImage(path, ion_image_);
        }
    }
}

Parser::ObjectSpec Image::GetObjectSpec() {
    SG::SpecBuilder<Image> builder;
    builder.AddString("path", &Image::path_);
    return Parser::ObjectSpec{
        "Image", false, []{ return new Image; }, builder.GetSpecs() };
}

}  // namespace SG
