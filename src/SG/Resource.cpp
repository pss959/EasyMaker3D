#include "SG/Scene.h"

#include "SG/SpecBuilder.h"

namespace SG {


std::vector<NParser::FieldSpec> Resource::GetFieldSpecs() {
    SG::SpecBuilder<Scene> builder;
    builder.AddString("path", &Resource::path_);
    return builder.GetSpecs();
}

}  // namespace SG
