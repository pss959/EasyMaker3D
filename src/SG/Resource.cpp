#include "SG/Scene.h"

#include "SG/SpecBuilder.h"

namespace SG {

NParser::ObjectSpec Resource::GetObjectSpec() {
    SG::SpecBuilder<Scene> builder;
    builder.AddString("path", &Resource::path_);
    return NParser::ObjectSpec{
        "Resource", false, []{ return new Resource; }, builder.GetSpecs() };
}

Util::FilePath Resource::GetFullPath(const std::string &subdir) const {
    Util::FilePath path(path_);
    if (path.IsAbsolute())
        return path;
    return Util::FilePath::GetResourcePath(subdir, path);
}

}  // namespace SG
