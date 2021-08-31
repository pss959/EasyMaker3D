#include "SG/Cylinder.h"

#include <algorithm>

#include <ion/gfx/attributearray.h>
#include <ion/gfx/bufferobject.h>
#include <ion/gfx/indexbuffer.h>
#include <ion/gfxutils/shapeutils.h>

#include "SG/SpecBuilder.h"

namespace SG {

Bounds Cylinder::ComputeBounds() const {
    const float max_diameter = 2.f * std::max(bottom_radius_, top_radius_);
    return Bounds(Vector3f(max_diameter, height_, max_diameter));
}

bool Cylinder::IntersectRay(const Ray &ray, Hit &hit) const {
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

ion::gfx::ShapePtr Cylinder::CreateIonShape() {
    ion::gfxutils::CylinderSpec spec;
    spec.bottom_radius    = bottom_radius_;
    spec.top_radius       = top_radius_;
    spec.height           = height_;
    spec.has_top_cap      = has_top_cap_;
    spec.has_bottom_cap   = has_bottom_cap_;
    spec.shaft_band_count = shaft_band_count_;
    spec.cap_band_count   = cap_band_count_;
    spec.sector_count     = sector_count_;
    // Need to access the attribute data.
    spec.usage_mode = ion::gfx::BufferObject::kDynamicDraw;
    ion::gfx::ShapePtr shape = ion::gfxutils::BuildCylinderShape(spec);
    FillTriMesh_(*shape);
    return shape;
}

Parser::ObjectSpec Cylinder::GetObjectSpec() {
    SG::SpecBuilder<Cylinder> builder;
    builder.AddFloat("bottom_radius",  &Cylinder::bottom_radius_);
    builder.AddFloat("top_radius",     &Cylinder::top_radius_);
    builder.AddFloat("height",         &Cylinder::height_);
    builder.AddBool("has_top_cap",     &Cylinder::has_top_cap_);
    builder.AddBool("has_bottom_cap",  &Cylinder::has_bottom_cap_);
    builder.AddInt("shaft_band_count", &Cylinder::shaft_band_count_);
    builder.AddInt("cap_band_count",   &Cylinder::cap_band_count_);
    builder.AddInt("sector_count",     &Cylinder::sector_count_);
    return Parser::ObjectSpec{
        "Cylinder", false, []{ return new Cylinder; }, builder.GetSpecs() };
}

void Cylinder::FillTriMesh_(const ion::gfx::Shape &shape) {
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
