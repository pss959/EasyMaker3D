#include "SG/TriMeshShape.h"

#include <ion/gfx/attributearray.h>
#include <ion/gfx/bufferobject.h>
#include <ion/gfx/indexbuffer.h>
#include <ion/gfxutils/shapeutils.h>

#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "SG/Hit.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

/// This struct is used to store all of the necessary data for generating
/// normals.
struct NormalData_ {
    size_t      count;    ///< Vertex count.
    size_t      icount;   ///< Index count.

    /// Function to access the position for an indexed vertex.
    std::function<const Point3f *(size_t)> pos_func;

    /// Function to access the writable normal for an indexed vertex.
    std::function<Vector3f *(size_t)>      norm_func;

    /// Function to access the vertex index from an IndexBuffer index.
    std::function<size_t(size_t)>          index_func;
};

/// Generates per-vertex normals from face normals.
static void GenerateFaceNormals_(ion::gfx::Shape &shape,
                                 const NormalData_ &data) {
    // Set face normal in each face vertex.
    for (size_t i = 0; i < data.icount; i += 3) {
        const int i0 = data.index_func(i);
        const int i1 = data.index_func(i + 1);
        const int i2 = data.index_func(i + 2);
        const Vector3f normal = ComputeNormal(*data.pos_func(i0),
                                              *data.pos_func(i1),
                                              *data.pos_func(i2));
        *data.norm_func(i0) = normal;
        *data.norm_func(i1) = normal;
        *data.norm_func(i2) = normal;
    }
}

/// Generates smooth vertex normals by averaging face normals.
static void GenerateVertexNormals_(ion::gfx::Shape &shape,
                                   const NormalData_ &data) {
    // Zero out all normals in case there is anything there.
    for (size_t i = 0; i < data.count; ++i)
        data.norm_func(i)->Set(0, 0, 0);

    // Add face normal to each face vertex.
    for (size_t i = 0; i < data.icount; i += 3) {
        const int i0 = data.index_func(i);
        const int i1 = data.index_func(i + 1);
        const int i2 = data.index_func(i + 2);
        const Vector3f normal = ComputeNormal(*data.pos_func(i0),
                                              *data.pos_func(i1),
                                              *data.pos_func(i2));
        *data.norm_func(i0) += normal;
        *data.norm_func(i1) += normal;
        *data.norm_func(i2) += normal;
    }

    // Normalize all of the vertex normals.
    for (size_t i = 0; i < data.count; ++i)
        ion::math::Normalize(data.norm_func(i));
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// TriMeshShape functions.
// ----------------------------------------------------------------------------

namespace SG {

bool TriMeshShape::IntersectRay(const Ray &ray, Hit &hit) const {
    float        distance;
    TriMesh::Hit tmhit;
    if (RayTriMeshIntersect(ray, tri_mesh_, distance, tmhit)) {
        hit.distance    = distance;
        hit.point       = tmhit.point;
        hit.normal      = tmhit.normal;
        hit.indices     = tmhit.indices;
        hit.barycentric = tmhit.barycentric;
        return true;
    }
    return false;
}

Bounds TriMeshShape::ComputeBounds() const {
    return ComputeMeshBounds(tri_mesh_);
}

// TODO: Clean up/simplify all of this...

void TriMeshShape::GenerateNormals(ion::gfx::Shape &shape, NormalType type) {
    using ion::gfx::BufferObject;
    using ion::gfx::BufferObjectElement;

    ASSERT(type != NormalType::kNoNormals);

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
    ASSERT(pbo.GetData());
    ASSERT(pbo.GetData()->GetData());
    ASSERT(pbo.GetCount() == nbo.GetCount());

    // Access the index buffer data. The index count must be a multiple of 3,
    // and there has to be data.
    const ion::gfx::IndexBuffer &ib = *shape.GetIndexBuffer();
    const size_t icount = ib.GetCount();
    ASSERT(icount % 3U == 0U);
    ASSERT(ib.GetData()->GetData());

    // The IndexBuffer may have short or int indices.
    const BufferObject::Spec &ispec = ib.GetSpec(0);
    ASSERT(! ion::base::IsInvalidReference(ispec));
    ASSERT(ispec.byte_offset == 0U);

    // Set up a NormalData_ instance.
    const char *pdata = static_cast<const char*>(pbo.GetData()->GetData());
    char       *ndata = nbo.GetData()->GetMutableData<char>();
    size_t      pstride = pbo.GetStructSize();
    size_t      nstride = nbo.GetStructSize();
    NormalData_ data;
    data.count      = pbo.GetCount();
    data.icount     = icount;
    data.pos_func   = [pdata, pstride, &pspec](size_t index){
        return reinterpret_cast<const Point3f *>(
            &pdata[pstride * index + pspec.byte_offset]); };
    data.norm_func  = [ndata, nstride, &nspec](size_t index){
        return reinterpret_cast<Vector3f *>(
            &ndata[nstride * index + nspec.byte_offset]); };
    data.index_func = [&ib, &ispec](size_t i) {
        if (ispec.type == BufferObject::kUnsignedShort)
            return static_cast<uint32>(ib.GetData()->GetData<uint16>()[i]);
        else
            return ib.GetData()->GetData<uint32>()[i];
    };

    if (type == NormalType::kVertexNormals)
        GenerateVertexNormals_(shape, data);
    else
        GenerateFaceNormals_(shape, data);
}

void TriMeshShape::GenerateTexCoords(ion::gfx::Shape &shape,
                                     TexCoordsType type) {
    int dim0, dim1;
    switch (type) {
      case TexCoordsType::kTexCoordsXY: dim0 = 0; dim1 = 1; break;
      case TexCoordsType::kTexCoordsXZ: dim0 = 0; dim1 = 2; break;
      case TexCoordsType::kTexCoordsYX: dim0 = 1; dim1 = 0; break;
      case TexCoordsType::kTexCoordsYZ: dim0 = 1; dim1 = 2; break;
      case TexCoordsType::kTexCoordsZX: dim0 = 2; dim1 = 0; break;
      case TexCoordsType::kTexCoordsZY: dim0 = 2; dim1 = 1; break;
      default: ASSERT(false);
    }

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
    tri_mesh_ = IonShapeToTriMesh(shape);
    ProcessChange(Change::kGeometry, *this);
}

void TriMeshShape::InstallMesh(const TriMesh &mesh) {
    tri_mesh_ = mesh;
    ProcessChange(Change::kGeometry, *this);
}

}  // namespace SG
