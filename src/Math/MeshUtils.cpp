#include "Math/MeshUtils.h"

#include <functional>
#include <numeric>

#include "ion/gfxutils/buffertoattributebinder.h"
#include <ion/gfx/attributearray.h>
#include <ion/gfx/bufferobject.h>
#include <ion/gfx/indexbuffer.h>
#include <ion/math/transformutils.h>

#include "Util/Assert.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

/// Creates a new mesh by modifying each point in the given mesh using the
/// given function. If change_orientation is true, this also changes the
/// orientation of the new mesh's triangles.
static TriMesh ModifyVertices_(
    const TriMesh &mesh, const std::function<Point3f(const Point3f &)> &func,
    bool change_orientation = false) {
    TriMesh new_mesh;

    const size_t pt_count  = mesh.points.size();
    new_mesh.points.resize(pt_count);
    for (size_t i = 0; i < pt_count; ++i)
        new_mesh.points[i] = func(mesh.points[i]);

    // Change the orientation of all triangles if requested.
    if (change_orientation) {
        const size_t tri_count = mesh.indices.size() / 3;
        new_mesh.indices.resize(3 * tri_count);
        for (size_t i = 0; i < tri_count; ++i) {
            new_mesh.indices[3 * i + 0] = mesh.indices[3 * i + 2];
            new_mesh.indices[3 * i + 1] = mesh.indices[3 * i + 1];
            new_mesh.indices[3 * i + 2] = mesh.indices[3 * i + 0];
        }
    }
    else {
        new_mesh.indices = mesh.indices;
    }
    return new_mesh;
}

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

TriMesh TransformMesh(const TriMesh &mesh, const Matrix4f &m) {
    return ModifyVertices_(mesh, [m](const Point3f &p){ return m * p; });
}

TriMesh MirrorMesh(const TriMesh &mesh, const Plane &plane) {
    return ModifyVertices_(mesh, [plane](const Point3f &p){
        return plane.MirrorPoint(p); }, true);
}

Vector3f CenterMesh(TriMesh &mesh) {
    const Bounds bounds = ComputeMeshBounds(mesh);
    const Vector3f offset = Vector3f(-bounds.GetCenter());
    if (offset != Vector3f::Zero()) {
        for (auto &p: mesh.points)
            p += offset;
    }
    return offset;
}

void UnshareMeshVertices(TriMesh &mesh) {
    std::vector<Point3f> unique_points;
    unique_points.reserve(mesh.indices.size());
    for (auto index: mesh.indices)
        unique_points.push_back(mesh.points[index]);
    mesh.points = unique_points;
    std::iota(mesh.indices.begin(), mesh.indices.end(), 0);
}

void CleanMesh(TriMesh &mesh) {
    auto clean_val = [](float &val){
        const float kTolerance = 1.e-5f;
        if (std::abs(val) <= kTolerance)
            val = 0;
    };
    for (auto &p: mesh.points) {
        clean_val(p[0]);
        clean_val(p[1]);
        clean_val(p[2]);
    }
}

ion::gfx::ShapePtr TriMeshToIonShape(const TriMesh &mesh, bool alloc_normals,
                                     bool alloc_tex_coords, bool is_dynamic) {
    ion::gfx::ShapePtr shape(new ion::gfx::Shape);
    UpdateIonShapeFromTriMesh(mesh, *shape,
                              alloc_normals, alloc_tex_coords, is_dynamic);
    return shape;
}

void UpdateIonShapeFromTriMesh(const TriMesh &mesh, ion::gfx::Shape &shape,
                               bool alloc_normals, bool alloc_tex_coords,
                               bool is_dynamic) {
    using ion::base::DataContainer;
    using ion::gfx::BufferObject;
    using ion::gfxutils::BufferToAttributeBinder;

    ion::base::AllocatorPtr   alloc;
    ion::gfx::BufferObjectPtr bo(new BufferObject);
    const BufferObject::UsageMode usage_mode =
        is_dynamic ? BufferObject::kDynamicDraw : BufferObject::kStaticDraw;
    const bool is_wipeable = ! is_dynamic;

    // Create a BufferObject to hold the vertex attributes and then build an
    // AttributeArray for them.
    ion::gfx::AttributeArrayPtr aa(new ion::gfx::AttributeArray);
    if (alloc_normals && alloc_tex_coords) {
        struct VertexPTN {
            Point3f  position{0, 0, 0};
            Point2f  texture_coords{0, 0};
            Vector3f normal{0, 0, 0};
        };
        std::vector<VertexPTN> verts(mesh.points.size());
        for (size_t i = 0; i < mesh.points.size(); ++i)
            verts[i].position = mesh.points[i];
        auto dc = DataContainer::CreateAndCopy<VertexPTN>(
            verts.data(), verts.size(), is_wipeable, alloc);
        bo->SetData(dc, sizeof(verts[0]), verts.size(), usage_mode);
        VertexPTN vertex;
        BufferToAttributeBinder<VertexPTN>(vertex)
            .Bind(vertex.position,       "aVertex")
            .Bind(vertex.texture_coords, "aTexCoords")
            .Bind(vertex.normal,         "aNormal")
            .Apply(ion::gfx::ShaderInputRegistry::GetGlobalRegistry(), aa, bo);
    }
    else if (alloc_normals) {
        struct VertexPN {
            Point3f  position{0, 0, 0};
            Vector3f normal{0, 0, 0};
        };
        std::vector<VertexPN> verts(mesh.points.size());
        for (size_t i = 0; i < mesh.points.size(); ++i)
            verts[i].position = mesh.points[i];
        auto dc = DataContainer::CreateAndCopy<VertexPN>(
            verts.data(), verts.size(), is_wipeable, alloc);
        bo->SetData(dc, sizeof(verts[0]), verts.size(), usage_mode);
        VertexPN vertex;
        BufferToAttributeBinder<VertexPN>(vertex)
            .Bind(vertex.position, "aVertex")
            .Bind(vertex.normal,   "aNormal")
            .Apply(ion::gfx::ShaderInputRegistry::GetGlobalRegistry(), aa, bo);
    }
    else if (alloc_tex_coords) {
        struct VertexPT {
            Point3f position{0, 0, 0};
            Point2f texture_coords{0, 0};
        };
        std::vector<VertexPT> verts(mesh.points.size());
        for (size_t i = 0; i < mesh.points.size(); ++i)
            verts[i].position = mesh.points[i];
        auto dc = DataContainer::CreateAndCopy<VertexPT>(
            verts.data(), verts.size(), is_wipeable, alloc);
        bo->SetData(dc, sizeof(verts[0]), verts.size(), usage_mode);
        VertexPT vertex;
        BufferToAttributeBinder<VertexPT>(vertex)
            .Bind(vertex.position,       "aVertex")
            .Bind(vertex.texture_coords, "aTexCoords")
            .Apply(ion::gfx::ShaderInputRegistry::GetGlobalRegistry(), aa, bo);
    }
    else {
        Point3f vertex;
        auto dc = DataContainer::CreateAndCopy<Point3f>(
            mesh.points.data(), mesh.points.size(), is_wipeable, alloc);
        bo->SetData(dc, sizeof(mesh.points[0]), mesh.points.size(), usage_mode);
        BufferToAttributeBinder<Point3f>(vertex)
            .Bind(vertex, "aVertex")
            .Apply(ion::gfx::ShaderInputRegistry::GetGlobalRegistry(), aa, bo);
    }

    // Build an IndexBuffer for the indices.
    ion::gfx::IndexBufferPtr ib(new ion::gfx::IndexBuffer);
    auto dc = DataContainer::CreateAndCopy(
        mesh.indices.data(), mesh.indices.size(), is_wipeable, alloc);
    ib->AddSpec(BufferObject::kUnsignedInt, 1, 0);
    ib->SetData(dc, sizeof(mesh.indices[0]), mesh.indices.size(), usage_mode);

    shape.SetPrimitiveType(ion::gfx::Shape::kTriangles);
    shape.SetAttributeArray(aa);
    shape.SetIndexBuffer(ib);
}

TriMesh IonShapeToTriMesh(const ion::gfx::Shape &shape) {
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
    TriMesh mesh;
    mesh.points.resize(count);
    for (size_t i = 0; i < count; ++i) {
        mesh.points[i] = *reinterpret_cast<const Point3f *>(
            &data[stride * i + spec.byte_offset]);
    }

    // Access the index buffer data if it exists.
    if (shape.GetIndexBuffer()) {
        const ion::gfx::IndexBuffer &ib = *shape.GetIndexBuffer();

        // The index count must be a multiple of 3, and there has to be data.
        const size_t icount = ib.GetCount();
        ASSERT(icount % 3U == 0U);
        ASSERT(ib.GetData()->GetData());

        // The IndexBuffer may have short or int indices.
        const BufferObject::Spec &ispec = ib.GetSpec(0);
        ASSERT(! ion::base::IsInvalidReference(ispec));
        ASSERT(ispec.byte_offset == 0U);
        if (ispec.type == BufferObject::kUnsignedShort) {
            const uint16 *indices = ib.GetData()->GetData<uint16>();
            mesh.indices.resize(icount);
            for (size_t i = 0; i < icount; ++i)
                mesh.indices[i] = indices[i];
        }
        else {
            ASSERT(ispec.type == BufferObject::kUnsignedInt);
            const uint32 *indices = ib.GetData()->GetData<uint32>();
            mesh.indices.resize(icount);
            for (size_t i = 0; i < icount; ++i)
                mesh.indices[i] = indices[i];
        }
    }
    else if (shape.GetPrimitiveType() == ion::gfx::Shape::kTriangleFan) {
        // The first point is the center of the fan. Every other pair of points
        // forms a triangle.
        mesh.indices.reserve(count - 1);
        for (size_t i = 1; i < count; ++i) {
            mesh.indices.push_back(0);
            mesh.indices.push_back(i - 1);
            mesh.indices.push_back(i);
        }
    }
    else {
        // Otherwise, set up sequential indices.
        ASSERT(count % 3 == 0);
        mesh.indices.resize(count);
        std::iota(mesh.indices.begin(), mesh.indices.end(), 0);
    }

    return mesh;
}

void WriteMeshAsOFF(const TriMesh &mesh, const std::string &description,
                    std::ostream &out) {
    out << "OFF\n";
    out << "# " << description << ":\n";
    const size_t tri_count = mesh.indices.size() / 3U;
    out << mesh.points.size() << ' ' << tri_count << " 0\n";
    for (size_t i = 0; i < mesh.points.size(); ++i) {
        const Point3f &p = mesh.points[i];
        out << p[0] << ' ' << p[1] << ' ' << p[2] << "\n";
    }
    for (size_t i = 0; i < tri_count; ++i) {
        const int i0 = mesh.indices[3 * i + 0];
        const int i1 = mesh.indices[3 * i + 1];
        const int i2 = mesh.indices[3 * i + 2];
        out << "3 " << i0 << ' ' << i1 << ' ' << i2 << "\n";
    }
}

Bounds ComputeMeshBounds(const TriMesh &mesh) {
    Bounds bounds;
    for (const Point3f &pt: mesh.points)
        bounds.ExtendByPoint(pt);
    return bounds;
}
