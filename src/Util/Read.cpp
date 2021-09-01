#include "Util/Read.h"

#include <fstream>

#include <ion/gfxutils/shapeutils.h>
#include <ion/image/conversionutils.h>
#include <ion/port/fileutils.h>

#include "Util/String.h"

namespace Util {

bool ReadFile(const FilePath &path, std::string &s) {
    return ion::port::ReadDataFromFile(path.ToString(), &s);
}

ion::gfx::ImagePtr ReadImage(const FilePath &path) {
    std::string data;
    ion::gfx::ImagePtr image;
    if (ReadFile(path, data)) {
        image = ion::image::ConvertFromExternalImageData(
            data.data(), data.size(), false, false, ion::base::AllocatorPtr());
    }
    return image;
}

ion::gfx::ShapePtr ReadShape(const FilePath &path) {
    using ion::gfxutils::ExternalShapeSpec;

    ion::gfx::ShapePtr shape;

    // Determine the format from the extension.
    const std::string &ext = path.GetExtension();

    ExternalShapeSpec spec;
    spec.format = ExternalShapeSpec::kUnknown;
    if (StringsEqualNoCase(ext, ".3ds"))
        spec.format = ExternalShapeSpec::k3ds;
    else if (StringsEqualNoCase(ext, ".dae"))
        spec.format = ExternalShapeSpec::kDae;
    else if (StringsEqualNoCase(ext, ".lwo"))
        spec.format = ExternalShapeSpec::kLwo;
    else if (StringsEqualNoCase(ext, ".obj"))
        spec.format = ExternalShapeSpec::kObj;
    else if (StringsEqualNoCase(ext, ".off"))
        spec.format = ExternalShapeSpec::kOff;

    // Continue if a supported format was detected.
    if (spec.format != ExternalShapeSpec::kUnknown) {
        spec.center_at_origin = true;
        // Most imported models do not have normals or texture coordinates.
        spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
        // Need to access the attribute data.
        spec.usage_mode = ion::gfx::BufferObject::kDynamicDraw;

        // Open the file.
        std::ifstream in(path);
        if (! in.fail())
            shape = ion::gfxutils::LoadExternalShape(spec, in);
    }
    return shape;
}

}  // namespace Util
