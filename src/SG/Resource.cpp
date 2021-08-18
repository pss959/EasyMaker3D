#include "SG/Scene.h"

#include "SG/SpecBuilder.h"

namespace SG {


NParser::ObjectSpec Resource::GetObjectSpec() {
    SG::SpecBuilder<Scene> builder;
    builder.AddString("path", &Resource::path_);
    return NParser::ObjectSpec{
        "Resource", []{ return new Resource; }, builder.GetSpecs() };
}

}  // namespace SG
