#include "SG/ImportedShape.h"

#include "Math/Intersection.h"
#include "Math/MeshUtils.h"
#include "Parser/Registry.h"
#include "SG/Exception.h"
#include "SG/Hit.h"
#include "Util/Read.h"

namespace SG {

void ImportedShape::AddFields() {
    AddField(path_.Init("path"));
    AddField(normal_type_.Init("normal_type", NormalType::kNoNormals));
    AddField(tex_coords_type_.Init("tex_coords_type",
                                   TexCoordsType::kNoTexCoords));
    AddField(proxy_shape_.Init("proxy_shape"));
    AddField(use_bounds_proxy_.Init("use_bounds_proxy", false));

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
    const FilePath path =
        FilePath::GetFullResourcePath("shapes", GetFilePath());

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
    if (normal_type_ == NormalType::kFaceNormals)
        GenerateFaceNormals(*shape);
    else if (normal_type_ == NormalType::kVertexNormals)
        GenerateVertexNormals(*shape);
    if (gen_tex_coords)
        GenerateTexCoords(tex_coords_type_, *shape);

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
