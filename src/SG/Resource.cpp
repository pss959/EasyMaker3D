#include "SG/Resource.h"

#include "SG/SpecBuilder.h"

namespace SG {

Parser::ObjectSpec Resource::GetObjectSpec() {
    SG::SpecBuilder<Resource> builder;
    builder.AddString("path", &Resource::path_);
    return Parser::ObjectSpec{
        "Resource", false, []{ return new Resource; }, builder.GetSpecs() };
}

Util::FilePath Resource::GetFullPath(const std::string &subdir) const {
    Util::FilePath path(path_);
    if (path.IsAbsolute())
        return path;
    return Util::FilePath::GetResourcePath(subdir, path);
}

}  // namespace SG
