#include "SG/ImportedShape.h"

#include "SG/Exception.h"
#include "SG/SpecBuilder.h"
#include "Util/Read.h"

namespace SG {

bool ImportedShape::IntersectRay(const Ray &ray, Hit &hit) const {
    // If there is a proxy shape, intersect with it. Otherwise, let the base
    // class handle it.
    if (proxy_shape_)
        return proxy_shape_->IntersectRay(ray, hit);
    else
        return TriMeshShape::IntersectRay(ray, hit);
}

ion::gfx::ShapePtr ImportedShape::CreateIonShape() {
    const Util::FilePath path =
        Util::FilePath::GetFullResourcePath("shapes", path_);

    ion::gfx::ShapePtr shape =
        Util::ReadShape(path, add_normals_, add_texcoords_);

    if (! shape)
        throw Exception("Unable to open or read shape file '" +
                        path.ToString() + "'");

    // Generate normals and tex coords if requested.
    if (add_normals_)
        GenerateVertexNormals(*shape);
    if (add_texcoords_)
        GenerateTexCoords(*shape, tex_dimensions_);

    FillTriMesh(*shape);

    return shape;
}

Parser::ObjectSpec ImportedShape::GetObjectSpec() {
    SG::SpecBuilder<ImportedShape> builder;
    builder.AddString("path",               &ImportedShape::path_);
    builder.AddBool("add_normals",          &ImportedShape::add_normals_);
    builder.AddBool("add_texcoords",        &ImportedShape::add_texcoords_);
    builder.AddVector2i("tex_dimensions",   &ImportedShape::tex_dimensions_);
    builder.AddObject<Shape>("proxy_shape", &ImportedShape::proxy_shape_);
    return Parser::ObjectSpec{
        "ImportedShape", false, []{ return new ImportedShape; },
        builder.GetSpecs() };
}

}  // namespace SG
