#include "SG/FileImage.h"

#include "SG/Exception.h"
#include "SG/Tracker.h"
#include "Util/Assert.h"
#include "Util/Read.h"

namespace SG {

void FileImage::AddFields() {
    AddField(path_);
    Image::AddFields();
}

void FileImage::SetFilePath(const Util::FilePath &path) {
    ASSERT(! GetIonImage());
    path_ = path.ToString();
}

ion::gfx::ImagePtr FileImage::CreateIonImage(Tracker &tracker) {
    ion::gfx::ImagePtr image;
    if (GetFilePath()) {
        const auto path =
            Util::FilePath::GetFullResourcePath("images", GetFilePath());

        // Check the Tracker first to see if the Image was already loaded.
        image = tracker.FindImage(path);
        if (! image) {
            image = Util::ReadImage(path);
            if (! image)
                throw Exception("Unable to open or read image file '" +
                                path.ToString() + "'");
            tracker.AddImage(path, image);
        }
    }
    return image;
}

}  // namespace SG
