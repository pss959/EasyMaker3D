#include "SG/ShaderSource.h"

#include <assert.h>

#include "NParser/Exception.h"
#include "Util/Read.h"

namespace SG {

void ShaderSource::SetUpIon(IonContext &context) {
    // XXXX Deal with Tracker!
    if (source_string_.empty()) {
        if (! Util::ReadFile(GetFilePath(), source_string_))
            throw NParser::Exception(GetFilePath(),
                                     "Unable to open or read shader file");
    }
}

NParser::ObjectSpec ShaderSource::GetObjectSpec() {
    return NParser::ObjectSpec{
        "ShaderSource", false, []{ return new ShaderSource; },
        Resource::GetObjectSpec().field_specs };
}

}  // namespace SG
