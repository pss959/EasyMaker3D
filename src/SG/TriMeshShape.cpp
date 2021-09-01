#include "SG/TriMeshShape.h"

#include <numeric>

#include <ion/gfx/attributearray.h>
#include <ion/gfx/bufferobject.h>
#include <ion/gfx/indexbuffer.h>
#include <ion/gfxutils/shapeutils.h>

#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "SG/SpecBuilder.h"

namespace SG {

Bounds TriMeshShape::ComputeBounds() const {
    Bounds bounds;
    for (const Point3f &pt: tri_mesh_.points)
        bounds.ExtendByPoint(pt);
    return bounds;
}

bool TriMeshShape::IntersectRay(const Ray &ray, Hit &hit) const {
    float        distance;
    TriMesh::Hit tmhit;
    if (RayTriMeshIntersect(ray, tri_mesh_, distance, tmhit)) {
        hit.distance = distance;
        hit.point    = tmhit.point;
        hit.normal   = tmhit.normal;
        hit.indices  = tmhit.indices;
        return true;
    }
    return false;
}

// TODO: Clean up/simplify all of this...

void TriMeshShape::GenerateVertexNormals(ion::gfx::Shape &shape) {
    using ion::gfx::BufferObject;
    using ion::gfx::BufferObjectElement;

    // Has to be triangles; has to have indices.
    ASSERT(shape.GetPrimitiveType() == ion::gfx::Shape::kTriangles);
    ASSERT(shape.GetIndexBuffer());

    const ion::gfx::AttributeArray &aa = *shape.GetAttributeArray();
    ASSERT(aa.GetAttributeCount()       >= 2U);  // At least positions/normals.
    ASSERT(aa.GetBufferAttributeCount() >= 2U);
    const int norm_index = aa.GetAttributeCount() == 3U ? 2 : 1;
    const ion::gfx::Attribute &pattr = aa.GetAttribute(0);
    const ion::gfx::Attribute &nattr = aa.GetAttribute(norm_index);
    ASSERT(pattr.Is<BufferObjectElement>());
    ASSERT(nattr.Is<BufferObjectElement>());

    // Access the buffer data.
    const BufferObjectElement &pboe  = pattr.GetValue<BufferObjectElement>();
    const BufferObjectElement &nboe  = nattr.GetValue<BufferObjectElement>();
    const BufferObject        &pbo   = *pboe.buffer_object;
    const BufferObject        &nbo   = *nboe.buffer_object;
    const BufferObject::Spec  &pspec = pbo.GetSpec(pboe.spec_index);
    const BufferObject::Spec  &nspec = nbo.GetSpec(nboe.spec_index);
    const char *pdata = static_cast<const char*>(pbo.GetData()->GetData());
    char       *ndata = nbo.GetData()->GetMutableData<char>();
    const size_t pstride = pbo.GetStructSize();
    const size_t nstride = nbo.GetStructSize();
    const size_t count = pbo.GetCount();
    ASSERT(nbo.GetCount() == count);

    // Zero out all normals in case there is anything there.
    for (size_t i = 0; i < count; ++i) {
        Vector3f &norm = *reinterpret_cast<Vector3f *>(
            &ndata[nstride * i + nspec.byte_offset]);
        norm.Set(0, 0, 0);
    }

    // Access the index buffer data. The index count must be a multiple of 3,
    // and there has to be data.
    const ion::gfx::IndexBuffer &ib = *shape.GetIndexBuffer();
    const size_t icount = ib.GetCount();
    ASSERT(icount % 3U == 0U);
    ASSERT(ib.GetData()->GetData());

    // The IndexBuffer has short indices.
    const BufferObject::Spec &ispec = ib.GetSpec(0);
    ASSERT(! ion::base::IsInvalidReference(ispec));
    ASSERT(ispec.byte_offset == 0U);
    ASSERT(ispec.type == BufferObject::kUnsignedShort);
    const uint16 *indices = ib.GetData()->GetData<uint16>();

    // Code simplifiers.
    auto pt_func = [pdata, pstride, pspec](int index){
        return *reinterpret_cast<const Point3f *>(
            &pdata[pstride * index + pspec.byte_offset]); };
    auto norm_func = [ndata, nstride, nspec](int index){
        return reinterpret_cast<Vector3f *>(
            &ndata[nstride * index + nspec.byte_offset]); };

    // Add face normal to each face vertex.
    for (size_t i = 0; i < icount; i += 3) {
        const int i0 = indices[i];
        const int i1 = indices[i + 1];
        const int i2 = indices[i + 2];
        const Vector3f normal =
            ComputeNormal(pt_func(i0), pt_func(i1), pt_func(i2));
        (*norm_func(i0)) += normal;
        (*norm_func(i1)) += normal;
        (*norm_func(i2)) += normal;
    }

    // Normalize all of the vertex normals.
    for (size_t i = 0; i < count; ++i)
        ion::math::Normalize(norm_func(i));
}

void TriMeshShape::GenerateTexCoords(ion::gfx::Shape &shape,
                                     const Vector2i &dimensions) {
    const int dim0 = dimensions[0];
    const int dim1 = dimensions[1];
    ASSERT(dim0 >= 0 && dim0 <= 2);
    ASSERT(dim1 >= 0 && dim1 <= 2);

    using ion::gfx::BufferObject;
    using ion::gfx::BufferObjectElement;

    // Has to be triangles; has to have indices.
    ASSERT(shape.GetPrimitiveType() == ion::gfx::Shape::kTriangles);
    ASSERT(shape.GetIndexBuffer());

    const ion::gfx::AttributeArray &aa = *shape.GetAttributeArray();
    ASSERT(aa.GetAttributeCount()       >= 2U); // At least positions/texcoords.
    ASSERT(aa.GetBufferAttributeCount() >= 2U);
    const ion::gfx::Attribute &pattr = aa.GetAttribute(0);
    const ion::gfx::Attribute &tattr = aa.GetAttribute(1);
    ASSERT(pattr.Is<BufferObjectElement>());
    ASSERT(tattr.Is<BufferObjectElement>());

    // Access the buffer data.
    const BufferObjectElement &pboe  = pattr.GetValue<BufferObjectElement>();
    const BufferObjectElement &tboe  = tattr.GetValue<BufferObjectElement>();
    const BufferObject        &pbo   = *pboe.buffer_object;
    const BufferObject        &tbo   = *tboe.buffer_object;
    const BufferObject::Spec  &pspec = pbo.GetSpec(pboe.spec_index);
    const BufferObject::Spec  &tspec = tbo.GetSpec(tboe.spec_index);
    const char *pdata = static_cast<const char*>(pbo.GetData()->GetData());
    char       *tdata = tbo.GetData()->GetMutableData<char>();
    const size_t pstride = pbo.GetStructSize();
    const size_t tstride = tbo.GetStructSize();
    const size_t count = pbo.GetCount();
    ASSERT(tbo.GetCount() == count);

    // Code simplifiers.
    auto pt_func = [pdata, pstride, pspec](int index){
        return *reinterpret_cast<const Point3f *>(
            &pdata[pstride * index + pspec.byte_offset]); };
    auto tc_func = [tdata, tstride, tspec](int index){
        return reinterpret_cast<Point2f *>(
            &tdata[tstride * index + tspec.byte_offset]); };

    // Compute min/max point value in each requested dimension.
    const float kMin =std::numeric_limits<float>::min();
    const float kMax =std::numeric_limits<float>::max();
    Vector2f min(kMax, kMax);
    Vector2f max(kMin, kMin);
    for (size_t i = 0; i < count; ++i) {
        const Point3f &pt = pt_func(i);
        min[0] = std::min(min[0], pt[dim0]);
        min[1] = std::min(min[1], pt[dim1]);
        max[0] = std::max(max[0], pt[dim0]);
        max[1] = std::max(max[1], pt[dim1]);
    }

    // Set texture coordinates.
    const Vector2f diff = max - min;
    for (size_t i = 0; i < count; ++i) {
        const Point3f &pt = pt_func(i);
        Point2f       &tc = *tc_func(i);
        tc.Set((pt[dim0] - min[0]) / diff[0], (pt[dim1] - min[1]) / diff[1]);
    }
}

void TriMeshShape::FillTriMesh(const ion::gfx::Shape &shape) {
    using ion::gfx::BufferObject;
    using ion::gfx::BufferObjectElement;

    // Vertex positions are always attribute 0.
    const ion::gfx::AttributeArray &aa   = *shape.GetAttributeArray();
    const ion::gfx::Attribute      &attr = aa.GetAttribute(0);
    ASSERT(attr.Is<BufferObjectElement>());

    // Access the vertex buffer data.
    const BufferObjectElement &boe  = attr.GetValue<BufferObjectElement>();
    const BufferObject        &bo   = *boe.buffer_object;
    const BufferObject::Spec  &spec = bo.GetSpec(boe.spec_index);
    const char *data = static_cast<const char*>(bo.GetData()->GetData());
    const size_t stride = bo.GetStructSize();
    const size_t count  = bo.GetCount();

    // Fill in the TriMesh points.
    tri_mesh_.points.resize(count);
    for (size_t i = 0; i < count; ++i) {
        tri_mesh_.points[i] = *reinterpret_cast<const Point3f *>(
            &data[stride * i + spec.byte_offset]);
    }

    // Access the index buffer data if it exists.
    if (shape.GetIndexBuffer()) {
        const ion::gfx::IndexBuffer &ib = *shape.GetIndexBuffer();

        // The index count must be a multiple of 3, and there has to be data.
        const size_t icount = ib.GetCount();
        ASSERT(icount % 3U == 0U);
        ASSERT(ib.GetData()->GetData());

        // The IndexBuffer has short indices.
        const BufferObject::Spec &ispec = ib.GetSpec(0);
        ASSERT(! ion::base::IsInvalidReference(ispec));
        ASSERT(ispec.byte_offset == 0U);
        ASSERT(ispec.type == BufferObject::kUnsignedShort);
        const uint16 *indices = ib.GetData()->GetData<uint16>();
        tri_mesh_.indices.resize(icount);
        for (size_t i = 0; i < icount; ++i)
            tri_mesh_.indices[i] = indices[i];
    }
    else if (shape.GetPrimitiveType() == ion::gfx::Shape::kTriangleFan) {
        // The first point is the center of the fan. Every other pair of points
        // forms a triangle.
        tri_mesh_.indices.reserve(count - 1);
        for (size_t i = 1; i < count; ++i) {
            tri_mesh_.indices.push_back(0);
            tri_mesh_.indices.push_back(i - 1);
            tri_mesh_.indices.push_back(i);
        }
    }
    else {
        // Otherwise, set up sequential indices.
        ASSERT(count % 3 == 0);
        tri_mesh_.indices.resize(count);
        std::iota(tri_mesh_.indices.begin(), tri_mesh_.indices.end(), 0);
    }
}

}  // namespace SG
