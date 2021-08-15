#include "Util/Read.h"

#include <ion/image/conversionutils.h>
#include <ion/port/fileutils.h>

namespace Util {

bool ReadFile(const FilePath &path, std::string &s) {
    return ion::port::ReadDataFromFile(path.ToString(), &s);
}

//! Reads the contents of an image file with the given path and returns an Ion
//! Image representing it. Returns a null pointer if the file could not be
//! read.
ion::gfx::ImagePtr ReadImage(const FilePath &path) {
    std::string data;
    ion::gfx::ImagePtr image;
    if (ReadFile(path, data)) {
        image = ion::image::ConvertFromExternalImageData(
            data.data(), data.size(), false, false, ion::base::AllocatorPtr());
    }
    return image;
}

}  // namespace Util
