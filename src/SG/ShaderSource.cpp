#include "SG/ShaderSource.h"

#include <assert.h>

#include "SG/Exception.h"
#include "SG/Tracker.h"
#include "Util/Read.h"

namespace SG {

void ShaderSource::SetUpIon(IonContext &context) {
    if (source_string_.empty()) {
        const Util::FilePath path = GetFullPath("shaders");

        // Check the Tracker first to see if the source was already loaded.
        source_string_ = context.tracker.FindString(path);

        if (source_string_.empty()) {
            if (! Util::ReadFile(path, source_string_))
                throw Exception("Unable to open or read shader file '" +
                                path.ToString() + "'");
            context.tracker.AddString(path, source_string_);
        }
    }
}

NParser::ObjectSpec ShaderSource::GetObjectSpec() {
    return NParser::ObjectSpec{
        "ShaderSource", false, []{ return new ShaderSource; },
        Resource::GetObjectSpec().field_specs };
}

}  // namespace SG
