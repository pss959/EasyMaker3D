#include "SG/ShaderSource.h"

#include <assert.h>

#include "NParser/Exception.h"
#include "Util/Read.h"

namespace SG {

void ShaderSource::Finalize() {
    assert(source_string_.empty());
    if (! Util::ReadFile(GetFilePath(), source_string_))
        throw NParser::Exception(GetFilePath(),
                                 "Unable to open or read shader file");
}

}  // namespace SG
