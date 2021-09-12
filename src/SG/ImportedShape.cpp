#include "SG/ImportedShape.h"

#include "Math/Intersection.h"
#include "SG/Exception.h"
#include "Util/Read.h"

namespace SG {

void ImportedShape::AddFields() {
    AddField(path_);
    AddField(add_normals_);
    AddField(add_texcoords_);
    AddField(tex_dimensions_);
    AddField(proxy_shape_);
    AddField(use_bounds_proxy_);
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

ion::gfx::ShapePtr ImportedShape::CreateIonShape() {
    const Util::FilePath path =
        Util::FilePath::GetFullResourcePath("shapes", GetFilePath());

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
