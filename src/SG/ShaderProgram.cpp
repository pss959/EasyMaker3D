#include "SG/ShaderProgram.h"

#include "SG/ShaderSource.h"
#include "SG/SpecBuilder.h"
#include "SG/UniformDef.h"

namespace SG {

void ShaderProgram::Finalize() {
#if XXXX
    assert(! ion_program_);
    ion_program_.Reset(new ion::gfx::ShaderProgram);
    if (vertex_source_)
        ...;
#endif
}

std::vector<NParser::FieldSpec> ShaderProgram::GetFieldSpecs() {
    SG::SpecBuilder<ShaderProgram> builder;
    builder.AddObjectList<UniformDef>("uniform_defs",
                                      &ShaderProgram::uniform_defs_);
    builder.AddObject<ShaderSource>("vertex_source",
                                    &ShaderProgram::vertex_source_);
    builder.AddObject<ShaderSource>("geometry_source",
                                    &ShaderProgram::geometry_source_);
    builder.AddObject<ShaderSource>("fragment_source",
                                    &ShaderProgram::fragment_source_);
    return builder.GetSpecs();
}

}  // namespace SG
