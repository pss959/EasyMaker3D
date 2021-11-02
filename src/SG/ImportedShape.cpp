#include "SG/ImportedShape.h"

#include "Math/Intersection.h"
#include "Math/MeshUtils.h"
#include "Parser/Registry.h"
#include "SG/Exception.h"
#include "Util/Read.h"

namespace SG {

void ImportedShape::AddFields() {
    AddField(path_);
    AddField(normal_type_);
    AddField(tex_coords_type_);
    AddField(proxy_shape_);
    AddField(use_bounds_proxy_);
    TriMeshShape::AddFields();
}

ImportedShapePtr ImportedShape::CreateFrom(const std::string &path,
                                           NormalType normal_type) {
    ImportedShapePtr imp = Parser::Registry::CreateObject<ImportedShape>();
    imp->path_        = path;
    imp->normal_type_ = normal_type;
    return imp;
}

bool ImportedShape::IntersectRay(const Ray &ray, Hit &hit) const {
    // If there is a proxy shape, intersect with it. Otherwise, let the base
    // class handle it.
    if (use_bounds_proxy_)
        return IntersectBounds_(ray, hit);
    else if (GetProxyShape())
        return GetProxyShape()->IntersectRay(ray, hit);
    else
        return TriMeshShape::IntersectRay(ray, hit);
}

Bounds ImportedShape::ComputeBounds() const {
    if (GetProxyShape())
        return GetProxyShape()->GetBounds();
    else
        return TriMeshShape::ComputeBounds();
}

ion::gfx::ShapePtr ImportedShape::CreateSpecificIonShape() {
    const Util::FilePath path =
        Util::FilePath::GetFullResourcePath("shapes", GetFilePath());

    const bool gen_normals    = normal_type_     != NormalType::kNoNormals;
    const bool gen_tex_coords = tex_coords_type_ != TexCoordsType::kNoTexCoords;

    ion::gfx::ShapePtr shape =
        Util::ReadShape(path, gen_normals, gen_tex_coords);

    if (! shape)
        throw Exception("Unable to open or read shape file '" +
                        path.ToString() + "'");

    // If generating face normals, make sure vertices are not shared by
    // converting to a TriMesh, unsharing vertices, and converting back.
    if (normal_type_ == NormalType::kFaceNormals) {
        TriMesh mesh = IonShapeToTriMesh(*shape);
        UnshareMeshVertices(mesh);
        shape = TriMeshToIonShape(mesh, true, gen_tex_coords);
    }

    // Generate normals and tex coords if requested.
    if (gen_normals)
        GenerateNormals(*shape, normal_type_);
    if (gen_tex_coords)
        GenerateTexCoords(*shape, tex_coords_type_);

    FillTriMesh(*shape);

    return shape;
}

bool ImportedShape::IntersectBounds_(const Ray &ray, Hit &hit) const {
    float        distance;
    Bounds::Face face;
    bool         is_entry;
    if (RayBoundsIntersectFace(ray, GetValidBounds(), distance,
                               face, is_entry) && is_entry) {
        hit.distance = distance;
        hit.point    = ray.GetPoint(distance);
        hit.normal   = Bounds::GetFaceNormal(face);
    }
    return false;
}

}  // namespace SG
