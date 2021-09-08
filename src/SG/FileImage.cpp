#include "SG/FileImage.h"

#include "SG/Exception.h"
#include "SG/Tracker.h"
#include "Util/Read.h"

namespace SG {

void FileImage::AddFields() {
    AddField(path_);
}

void FileImage::SetUpIon(const ContextPtr &context) {
    Image::SetUpIon(context);
    if (! GetIonImage()) {
        const Util::FilePath path =
            Util::FilePath::GetFullResourcePath("images", GetFilePath());

        // Check the Tracker first to see if the Image was already loaded.
        ion::gfx::ImagePtr image = context->tracker->FindImage(path);

        if (! image) {
            image = Util::ReadImage(path);
            if (! image)
                throw Exception("Unable to open or read image file '" +
                                path.ToString() + "'");
            context->tracker->AddImage(path, image);
        }
        SetIonImage(image);
    }
}

}  // namespace SG
