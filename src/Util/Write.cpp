#include "Util/Write.h"

#include <fstream>

#include <stblib/stb_image_write.h>

#include "Util/FilePath.h"

namespace Util {

bool WriteString(const FilePath &path, const std::string &s) {
    std::ofstream out(path.ToNativeString());
    if (! out)
        return false;
    out << s;
    return true;
}

bool WriteImage(const FilePath &path, const ion::gfx::Image &image,
                bool flip_vertically) {
    const auto w = image.GetWidth();
    const auto h = image.GetHeight();
    const auto c = image.GetNumComponentsForFormat(image.GetFormat());
    const void *pp =
        reinterpret_cast<const void *>(image.GetData()->GetData<uint8>());

    stbi_flip_vertically_on_write(flip_vertically);
    return stbi_write_jpg(path.ToString().c_str(), w, h, c, pp, 100);
}

}  // namespace Util
