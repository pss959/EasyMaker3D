#include "Math/MeshUtils.h"

#include <functional>
#include <numeric>
#include <unordered_map>

#include <ion/gfx/attributearray.h>
#include <ion/gfx/bufferobject.h>
#include <ion/gfx/indexbuffer.h>
#include <ion/gfxutils/buffertoattributebinder.h>
#include <ion/math/transformutils.h>

#include "Math/Bend.h"
#include "Math/Linear.h"
#include "Math/Point3fMap.h"
#include "Math/SlicedMesh.h"
#include "Math/Taper.h"
#include "Math/Twist.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/KLog.h"
#include "Util/Tuning.h"

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
        const size_t tri_count = mesh.GetTriangleCount();
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

/// Given a Taper and a Y value in the range [0,1], this returns the X value
/// corresponding to the Taper scale.
static float GetTaperScale_(const Taper &taper, float y) {
    const auto &prof_pts = taper.profile.GetPoints();

    float scale = -1;
    // Special case for Y=1.
    if (y == 1) {
        scale = prof_pts[0][0];
    }
    else {
        // Otherwise, do a linear search. The Profile should never be so
        // big that it really matters how this search is done.
        for (size_t i = 1; i < prof_pts.size(); ++i) {
            // Profile points go from Y=1 at the beginning to Y=0 at the end.
            const Point2f &prev = prof_pts[i - 1];
            const Point2f &next = prof_pts[i];
            if (y < prev[1] && y >= next[1]) {
                scale = Lerp((y - next[1]) / (prev[1] - next[1]),
                             next[0], prev[0]);
                break;
            }
        }
    }
    ASSERT(scale >= 0);
    return scale;
}

/// Returns the area of the i'th triangle in a TriMesh.
static float GetTriangleArea_(const TriMesh &mesh, size_t i) {
    ASSERT(i < mesh.GetTriangleCount());

    const auto i0 = mesh.indices[3 * i + 0];
    const auto i1 = mesh.indices[3 * i + 1];
    const auto i2 = mesh.indices[3 * i + 2];

    return ComputeArea(mesh.points[i0], mesh.points[i1], mesh.points[i2]);
}

/// Removes all triangles with the given indices from a TriMesh.
static void RemoveIndexedTriangles_(TriMesh &mesh,
                                    const std::vector<size_t> &tris) {
    // Sort the triangle indices increasing to speed up the search.
    std::vector<size_t> sorted_tris = tris;
    std::sort(std::begin(sorted_tris), std::end(sorted_tris));

    // Construct a new_indices vector without the triangles to remove.
    std::vector<GIndex> new_indices;
    const size_t new_count = mesh.indices.size() - 3 * sorted_tris.size();
    new_indices.reserve(new_count);

    int next_tri = 0;
    for (size_t i = 0; i < mesh.GetTriangleCount(); ++i) {
        if (next_tri >= 0 && i == sorted_tris[next_tri]) {
            // If no more triangles to remove
            if (static_cast<size_t>(++next_tri) == sorted_tris.size())
                next_tri = -1;
        }
        else {
            for (int j = 0; j < 3; ++j)
                new_indices.push_back(mesh.indices[3 * i + j]);
        }
    }
    ASSERT(new_indices.size() == new_count);
    mesh.indices = new_indices;
}

static void RemoveUnusedPoints_(TriMesh &mesh) {
    const size_t point_count = mesh.points.size();

    std::vector<bool> is_used(point_count, false);

    size_t num_used = 0;
    for (size_t i = 0; i < mesh.GetTriangleCount(); ++i) {
        for (int j = 0; j < 3; ++j) {
            const GIndex index = mesh.indices[3 * i + j];
            if (! is_used[index]) {
                ++num_used;
                is_used[index] = true;
            }
        }
    }

    // Nothing to do if all of the points are used.
    if (num_used == point_count)
        return;

    // Copy all used points and keep track of how many points have been removed
    // before each one.
    std::vector<Point3f> remaining_pts;
    std::vector<size_t>  remove_count;
    remaining_pts.reserve(point_count);
    remove_count.reserve(point_count);

    size_t removed = 0;
    for (size_t i = 0; i < point_count; ++i) {
        if (is_used[i])
            remaining_pts.push_back(mesh.points[i]);
        else
            ++removed;
        remove_count.push_back(removed);
    }
    mesh.points  = remaining_pts;

    // Reindex.
    for (auto &index: mesh.indices)
        index -= remove_count[index];
}

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

TriMesh ScaleMesh(const TriMesh &mesh, const Vector3f &scale) {
    const auto scale_pt = [&scale](const Point3f &p){
        return Point3f(scale[0] * p[0], scale[1] * p[1], scale[2] * p[2]);
    };
    return ModifyVertices_(mesh, scale_pt);
}

TriMesh RotateMesh(const TriMesh &mesh, const Rotationf &rot) {
    return ModifyVertices_(mesh, [&rot](const Point3f &p){ return rot * p; });
}

TriMesh TransformMesh(const TriMesh &mesh, const Matrix4f &m) {
    return ModifyVertices_(mesh, [&m](const Point3f &p){ return m * p; });
}

TriMesh BendMesh(const SlicedMesh &sliced_mesh, const Bend &bend) {
    // If the angle is 0, there is no bending.
    if (bend.angle.Radians() == 0)
        return sliced_mesh.mesh;

    // Axis along which the mesh is sliced.
    const Vector3f slice_axis = GetAxis(sliced_mesh.axis);

    // Vector perpendicular to the slice axis and bend axis. This is used
    // to find the center of the bend circle.
    const Vector3f perp_vec = ion::math::Cross(bend.axis, slice_axis);

    // Get the plane perpendicular to the slicing axis that passes through the
    // bend center. All mesh points on this plane stay put; all other mesh
    // points rotate proportional to their distance from this plane.
    const Plane plane(bend.center, slice_axis);

    // If the plane is closer to the maximum end of the slice axis, negate the
    // rotation angle for consistency.
    const float angle_sign =
        SignedDistance(bend.center, perp_vec) > 0 ? -1 : 1;

    // If the bend offset is 0, don't let the angle exceed 360 degrees in
    // either direction.
    const Anglef angle = angle_sign *
        (bend.offset != 0 ? bend.angle :
         Anglef::FromDegrees(Clamp(bend.angle.Degrees(), -360.f, 360.f)));

    // Radius of bend circle, preserving arc length along the slicing axis.
    const float radius = sliced_mesh.range.GetSize() / angle.Radians();

    // Center point around which all mesh points are rotated.
    const Point3f center = bend.center + radius * perp_vec;

    const auto bend_pt = [&](const Point3f &p){
        // The relative distance of the point from the center in the direction
        // of the bend axis should not change. Compute the offset vector.
        const auto axis_offset =
            ion::math::Dot(p - center, bend.axis) * bend.axis;

        // The amount to rotate is proportional to the distance of the point
        // from the plane.
        const float rotate_scale =
            plane.GetDistanceToPoint(p) / sliced_mesh.range.GetSize();

        // Get the rotation by the scaled angle around the axis.
        const Rotationf rot =
            Rotationf::FromAxisAndAngle(bend.axis, rotate_scale * angle);

        // Find the radius to use based on the point's location along the
        // perpendicular vector from the bend center.
        const float rad = radius - ion::math::Dot(p - bend.center, perp_vec);

        // This vector is rotated around the circle center by the scaled
        // rotation to get the result mesh point.
        const Vector3f rot_vec = bend.center - (bend.center + rad * perp_vec);

        // Rotate about the bend axis through the center point.
        return center + rot * rot_vec + axis_offset;
    };

    TriMesh result_mesh = ModifyVertices_(sliced_mesh.mesh, bend_pt);
    CleanMesh(result_mesh);

    // If the angle is +/- 360, there is a good chance that there are now
    // duplicate triangles facing in opposite directions. Remove them if there
    // are any.
    if (AreClose(std::abs(angle.Degrees()), 360))
        RemoveDualTriangles(result_mesh);

    // The mesh turns inside out if the distance of the bend center to the
    // slicing axis is more than the radius. Detect this case and fix it by
    // reversing the orientation of all triangles.
    const float center_dist = SignedDistance(bend.center, perp_vec);
    const bool is_inside_out =
        radius > 0 ? (center_dist + radius < 0) : (center_dist + radius > 0);
    if (is_inside_out) {
        const size_t tri_count = result_mesh.GetTriangleCount();
        for (size_t i = 0; i < tri_count; ++i)
            std::swap(result_mesh.indices[3 * i + 1],
                      result_mesh.indices[3 * i + 2]);
    }

    return result_mesh;
}

TriMesh MirrorMesh(const TriMesh &mesh, const Plane &plane) {
    return ModifyVertices_(mesh, [&plane](const Point3f &p){
        return plane.MirrorPoint(p); }, true);
}

TriMesh TaperMesh(const SlicedMesh &sliced_mesh, const Taper &taper) {
    ASSERT(sliced_mesh.axis == taper.axis);
    ASSERT(Taper::IsValidProfile(taper.profile));
    const int   dim   = Util::EnumInt(taper.axis);
    const float min  = sliced_mesh.range.GetMinPoint();
    const float size = sliced_mesh.range.GetSize();

    const auto taper_pt = [&](const Point3f &p){
        // Scale the other 2 dimensions.
        const float scale = GetTaperScale_(taper, (p[dim] - min) / size);
        Point3f scaled_p = scale * p;
        scaled_p[dim] = p[dim];
        return scaled_p;
    };

    return ModifyVertices_(sliced_mesh.mesh, taper_pt);
}

TriMesh TwistMesh(const SlicedMesh &sliced_mesh, const Twist &twist) {
    const float min  = sliced_mesh.range.GetMinPoint();
    const float size = sliced_mesh.range.GetSize();

    const auto twist_pt = [&](const Point3f &p){
        // Interpolate the angle based on the distance.
        const float dist = ion::math::Dot(Vector3f(p), twist.axis);
        const Anglef angle = Lerp((dist - min) / size, Anglef(), twist.angle);
        const Rotationf rot = Rotationf::FromAxisAndAngle(twist.axis, angle);
        return twist.center + rot * (p - twist.center);
    };

    return ModifyVertices_(sliced_mesh.mesh, twist_pt);
}

TriMesh CenterMesh(const TriMesh &mesh) {
    TriMesh centered = mesh;
    const Bounds bounds = ComputeMeshBounds(centered);
    const Vector3f offset = Vector3f(-bounds.GetCenter());
    if (offset != Vector3f::Zero()) {
        for (auto &p: centered.points)
            p += offset;
    }
    return centered;
}

void UnshareMeshVertices(TriMesh &mesh) {
    std::vector<Point3f> unique_points;
    unique_points.reserve(mesh.indices.size());
    for (auto index: mesh.indices)
        unique_points.push_back(mesh.points[index]);
    mesh.points = unique_points;
    std::iota(mesh.indices.begin(), mesh.indices.end(), 0);
}

void RemoveDualTriangles(TriMesh &mesh) {
    // Maps a key representing vertex indices to triangle index.
    std::unordered_map<size_t, GIndex> tri_map;

    // Generates a size_t key from 3 vertex indices; the number of points is
    // used to guarantee uniqueness.
    const auto get_key = [&mesh](size_t tri_index){
        // Get the 3 vertex indices and sort increasing.
        GIndex ind[3];
        for (int i = 0; i < 3; ++i)
            ind[i] = mesh.indices[3 * tri_index + i];
        std::sort(std::begin(ind), std::end(ind));

        // Create a unique key.
        const size_t np = mesh.points.size();
        return ind[0] * np * np + ind[1] * np + ind[2];
    };

    // Collect indices of bad triangles.
    const size_t tri_count = mesh.GetTriangleCount();
    std::vector<size_t> bad_tris;
    for (size_t i = 0; i < tri_count; ++i) {
        const size_t key = get_key(i);
        const auto it = tri_map.find(key);
        if (it != tri_map.end()) {
            bad_tris.push_back(it->second);
            bad_tris.push_back(i);
        }
        else {
            tri_map[key] = i;
        }
    }

    // Remove bad triangles if any.
    if (! bad_tris.empty()) {
        RemoveIndexedTriangles_(mesh, bad_tris);
        RemoveUnusedPoints_(mesh);
    }
}

void CleanMesh(TriMesh &mesh) {
    KLOG('0', "Cleaning a mesh with " << mesh.points.size() << " points and "
         << mesh.GetTriangleCount() << " triangles");

    // First, use a Point3fMap to clean and uniquify points. Keep track of how
    // old indices map to new ones.
    Point3fMap point_map(TK::kMeshCleanPrecision);
    std::vector<GIndex> index_map;
    index_map.reserve(mesh.indices.size());
    for (size_t i = 0; i < mesh.points.size(); ++i) {
        const auto &p = mesh.points.at(i);
#if ENABLE_DEBUG_FEATURES
        const size_t prev_count = point_map.GetCount();
#endif
        index_map.push_back(point_map.Add(p));
#if ENABLE_DEBUG_FEATURES
        if (point_map.GetCount() == prev_count)
            KLOG('0', "   Point " << i
                 << " is the same as map point " << index_map.back());
#endif
    }

    // Replace the TriMesh contents, replacing the old indices with new ones.
    mesh.points = point_map.GetPoints();
    for (auto &index: mesh.indices)
        index = index_map[index];

    // Next, look for degenerate triangles.  Construct a vector of new indices
    // without degenerate triangles.
    const size_t tri_count = mesh.GetTriangleCount();
    std::vector<GIndex> new_indices;
    new_indices.reserve(mesh.indices.size());
    for (size_t i = 0; i < tri_count; ++i) {
        if (GetTriangleArea_(mesh, i) > 0) {
            new_indices.push_back(mesh.indices[3 * i + 0]);
            new_indices.push_back(mesh.indices[3 * i + 1]);
            new_indices.push_back(mesh.indices[3 * i + 2]);
        }
        else {
            KLOG('0', "   Triangle " << i << " is degenerate");
        }
    }
    if (new_indices.size() < mesh.indices.size()) {
        mesh.indices = new_indices;
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
    const size_t tri_count = mesh.GetTriangleCount();
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
