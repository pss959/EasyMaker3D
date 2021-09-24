#include "Util/Read.h"

#include <fstream>

#include <ion/gfxutils/shapeutils.h>
#include <ion/image/conversionutils.h>
#include <ion/port/fileutils.h>

#include "Util/String.h"

namespace Util {

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

/// Replaces \@include directives in the string with the contents of the
/// included file. Returns false on error.
static bool ReplaceIncludes_(const FilePath &base_path, std::string &s) {
    const std::string inc_str = "@include ";
    while (true) {
        const size_t pos = s.find(inc_str);
        if (pos == std::string::npos)
            break;

        // Find the quoted file path.
        size_t q0_pos = pos + inc_str.size();
        while (q0_pos < s.size() && isspace(s[q0_pos]))
            ++q0_pos;
        // If nothing after @include or missing quotes, bad.
        if (q0_pos == s.size() || s[q0_pos] != '"')
            return false;
        // Find close quote.
        size_t q1_pos = q0_pos + 1;
        while (q1_pos < s.size() && s[q1_pos] != '"')
            ++q1_pos;
        // If missing end quote, bad.
        if (q1_pos == s.size() || s[q1_pos] != '"')
            return false;
        const std::string path_str = s.substr(q0_pos + 1, q1_pos - q0_pos - 1);

        // Construct a relative path if necessary.
        const FilePath path = FilePath(path_str).MakeRelativeTo(base_path);

        // Read the file and replace the string.
        std::string contents;
        if (! ReadFile(path, contents, true))
            return false;
        s.replace(pos, q1_pos - pos + 1, contents);
    }
    return true;
}

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

bool ReadFile(const FilePath &path, std::string &s, bool allow_includes) {
    bool ok = ion::port::ReadDataFromFile(path.ToString(), &s);
    if (ok && allow_includes)
        ok = ReplaceIncludes_(path, s);
    return ok;
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

ion::gfx::ShapePtr ReadShape(const FilePath &path,
                             bool use_normals, bool use_tex_coords) {
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

        if (use_normals) {
            if (use_tex_coords)
                spec.vertex_type = ExternalShapeSpec::kPositionTexCoordsNormal;
            else
                spec.vertex_type = ExternalShapeSpec::kPositionNormal;
        }
        else if (use_tex_coords) {
            spec.vertex_type = ExternalShapeSpec::kPositionTexCoords;
        }
        else {
            spec.vertex_type = ExternalShapeSpec::kPosition;
        }

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
