#include "SG/FileImage.h"

#include "SG/Exception.h"
#include "SG/FileMap.h"
#include "Util/Assert.h"
#include "Util/Read.h"

namespace SG {

void FileImage::AddFields() {
    AddField(path_.Init("path"));

    Image::AddFields();
}

void FileImage::SetFilePath(const FilePath &path) {
    ASSERT(! GetIonImage());
    path_ = path.ToString();
}

ion::gfx::ImagePtr FileImage::CreateIonImage(FileMap &file_map) {
    ion::gfx::ImagePtr image;
    if (GetFilePath()) {
        const auto path =
            FilePath::GetFullResourcePath("images", GetFilePath());

        // Check the FileMap first to see if the Image was already loaded.
        image = file_map.FindImage(path);
        if (! image) {
            image = Util::ReadImage(path, true);  // Flip vertically.
            if (! image)
                throw Exception("Unable to open or read image file '" +
                                path.ToString() + "'");
            file_map.AddImage(path, image);
        }
    }
    return image;
}

}  // namespace SG
