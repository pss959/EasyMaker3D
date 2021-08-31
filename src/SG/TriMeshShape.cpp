#include "SG/TriMeshShape.h"

#include <algorithm>

#include <ion/gfx/attributearray.h>
#include <ion/gfx/bufferobject.h>
#include <ion/gfx/indexbuffer.h>
#include <ion/gfxutils/shapeutils.h>

#include "SG/SpecBuilder.h"

namespace SG {

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

    // Access the index buffer data.
    ASSERT(shape.GetIndexBuffer());
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

}  // namespace SG
