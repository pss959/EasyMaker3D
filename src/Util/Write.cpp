//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Util/Write.h"

#include <fstream>

#if ! defined(ION_PLATFORM_WINDOWS)
// STB stuff does not compile well on Windows.
#include <stblib/stb_image_write.h>
#endif

#include "Util/FilePath.h"

namespace Util {

bool WriteString(const FilePath &path, const Str &s) {
    // Use binary to work around line ending issues on Windows.
    std::ofstream out(path.ToNativeString(), std::ios::binary);
    if (! out)
        return false;
    out << s;
    return true;
}

bool WriteImage(const FilePath &path, const ion::gfx::Image &image,
                bool flip_vertically) {
#ifdef ION_PLATFORM_WINDOWS
    std::cerr << "*** Util::WriteImage() not implemented on Windows!\n";
    return false;
#else
    const auto w = image.GetWidth();
    const auto h = image.GetHeight();
    const auto c = image.GetNumComponentsForFormat(image.GetFormat());
    const void *pp =
        reinterpret_cast<const void *>(image.GetData()->GetData<uint8>());

    stbi_flip_vertically_on_write(flip_vertically);
    return stbi_write_jpg(path.ToString().c_str(), w, h, c, pp, 100);
#endif
}

}  // namespace Util
