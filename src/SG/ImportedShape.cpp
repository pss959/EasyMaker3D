#include "SG/ImportedShape.h"

#include "SG/Exception.h"
#include "SG/SpecBuilder.h"
#include "Util/Read.h"

namespace SG {

ion::gfx::ShapePtr ImportedShape::CreateIonShape() {
    const Util::FilePath path =
        Util::FilePath::GetFullResourcePath("shapes", path_);

    ion::gfx::ShapePtr shape = Util::ReadShape(path);
    if (! shape)
        throw Exception("Unable to open or read shape file '" +
                        path.ToString() + "'");

    FillTriMesh(*shape);
    return shape;
}

Parser::ObjectSpec ImportedShape::GetObjectSpec() {
    SG::SpecBuilder<ImportedShape> builder;
    builder.AddString("path", &ImportedShape::path_);
    return Parser::ObjectSpec{
        "ImportedShape", false, []{ return new ImportedShape; },
        builder.GetSpecs() };
}

}  // namespace SG
