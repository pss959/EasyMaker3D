#include "SG/FileImage.h"

#include "SG/Exception.h"
#include "SG/SpecBuilder.h"
#include "SG/Tracker.h"
#include "Util/Read.h"

namespace SG {

void FileImage::SetUpIon(IonContext &context) {
    if (! GetIonImage()) {
        const Util::FilePath path =
            Util::FilePath::GetFullResourcePath("images", path_);

        // Check the Tracker first to see if the Image was already loaded.
        ion::gfx::ImagePtr image = context.tracker.FindImage(path);

        if (! image) {
            image = Util::ReadImage(path);
            if (! image)
                throw Exception("Unable to open or read image file '" +
                                path.ToString() + "'");
            context.tracker.AddImage(path, image);
        }
        SetIonImage(image);
    }
}

Parser::ObjectSpec FileImage::GetObjectSpec() {
    SG::SpecBuilder<FileImage> builder;
    builder.AddString("path", &FileImage::path_);
    return Parser::ObjectSpec{
        "FileImage", false, []{ return new FileImage; }, builder.GetSpecs() };
}

}  // namespace SG
