//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/TriMeshShape.h"

#include <ion/gfx/attributearray.h>
#include <ion/gfx/bufferobject.h>
#include <ion/gfx/indexbuffer.h>
#include <ion/gfxutils/shapeutils.h>

#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "SG/Hit.h"

namespace {

// ----------------------------------------------------------------------------
// BufferObjectElement helper classes.
// ----------------------------------------------------------------------------

/// Helper class base for accessing data from a BufferObjectElement.
class BufferHelper_ {
  public:
    size_t GetCount() const { return count_; }

  protected:
    explicit BufferHelper_(const ion::gfx::BufferObjectElement &boe) :
        count_(boe.buffer_object->GetCount()),
        stride_(boe.buffer_object->GetStructSize()),
        offset_(boe.buffer_object->GetSpec(boe.spec_index).byte_offset) {}

    size_t GetLocation(int index) const { return stride_ * index + offset_; }

  private:
    const size_t count_;
    const size_t stride_;
    const size_t offset_;
};

/// Helper class that accesses const data from a BufferObjectElement.
class ConstBufferHelper_ : public BufferHelper_ {
  public:
    explicit ConstBufferHelper_(const ion::gfx::BufferObjectElement &boe) :
        BufferHelper_(boe),
        data_(static_cast<const char *>(
                  boe.buffer_object->GetData()->GetData())) {}

    /// Returns the indexed const typed object from the buffer data.
    template <typename T> T GetData(int index) const {
        return *reinterpret_cast<const T *>(&data_[GetLocation(index)]);
    }

  private:
    const char *data_;
};

/// Helper class that accesses mutable data from a BufferObjectElement.
class MutableBufferHelper_ : public BufferHelper_ {
  public:
    explicit MutableBufferHelper_(const ion::gfx::BufferObjectElement &boe) :
        BufferHelper_(boe),
        data_(boe.buffer_object->GetData()->GetMutableData<char>()) {}

    /// Returns the indexed mutable typed object from the buffer data.
    template <typename T> T & GetData(int index) const {
        return *reinterpret_cast<T *>(&data_[GetLocation(index)]);
    }

  private:
    char *data_;
};

/// Helper class that accesses const data from an IndexBuffer.
class IndexBufferHelper_ {
  public:
    explicit IndexBufferHelper_(const ion::gfx::IndexBuffer &ib);

    size_t GetCount() const { return count_; }

    /// Returns the indexed index.
    uint32 GetIndex(int index) const {
        return data16_ ? static_cast<uint32>(data16_[index]) : data32_[index];
    }

  private:
    const size_t  count_;
    const uint16 *data16_;  ///< Null if indices are 32-bit.
    const uint32 *data32_;  ///< Null if indices are 16-bit.
};

IndexBufferHelper_::IndexBufferHelper_(const ion::gfx::IndexBuffer &ib) :
    count_(ib.GetCount()) {
    // The count must be a multiple of 3 and there has to be data.
    ASSERT(count_ % 3U == 0U);

    // May have short or int indices.
    const auto &ispec = ib.GetSpec(0);
    ASSERT(! ion::base::IsInvalidReference(ispec));
    ASSERT(ispec.byte_offset == 0U);
    if (ispec.type == ion::gfx::BufferObject::kUnsignedShort) {
        data16_ = ib.GetData()->GetData<uint16>();
        data32_ = nullptr;
    }
    else {
        data16_ = nullptr;
        data32_ = ib.GetData()->GetData<uint32>();
    }
    // There has to be data.
    ASSERT(data16_ || data32_);
}

// ----------------------------------------------------------------------------
// ShapeHelper_ class.
// ----------------------------------------------------------------------------

/// Class that manages all relevant buffer helpers for an Ion Shape and
/// implements operations on them.
class ShapeHelper_ {
  public:
    /// The constructor is passed the Ion Shape and flags indicating whether to
    /// require normals and/or texture coordinates. At least one of these is
    /// required to be true.
    ShapeHelper_(ion::gfx::Shape &shape,
                 bool need_normals, bool need_tex_coords);

    void SetFaceNormals(const std::vector<Vector3f> &normals);
    void SetVertexNormals(const std::vector<Vector3f> &normals);
    void SetTextureCoords(const std::vector<Point2f> &tex_coords);

    void GenerateFaceNormals();
    void GenerateVertexNormals();
    void GenerateTexCoords(SG::TriMeshShape::TexCoordsType type);

  private:
    std::unique_ptr<ConstBufferHelper_>   pbh_;  ///< Positions.
    std::unique_ptr<MutableBufferHelper_> nbh_;  ///< Normals.
    std::unique_ptr<MutableBufferHelper_> tbh_;  ///< Tex coords.
    std::unique_ptr<IndexBufferHelper_>   ibh_;  ///< Indices.

    /// Returns the two dimensions to use for texture coordinates.
    static void GetTextureDimensions_(SG::TriMeshShape::TexCoordsType type,
                                      int &dim0, int &dim1);

    /// Computes the min/max bounds in the 2 given dimensions of all points.
    Range2f GetPointBounds_(int dim0, int dim1) const;
};

ShapeHelper_::ShapeHelper_(ion::gfx::Shape &shape,
                           bool need_normals, bool need_tex_coords) {
    using BOE = ion::gfx::BufferObjectElement;

    ASSERT(need_normals || need_tex_coords);

    // Has to be triangles; has to have indices.
    ASSERT(shape.GetPrimitiveType() == ion::gfx::Shape::kTriangles);
    ASSERT(shape.GetIndexBuffer());

    // Access points.
    const ion::gfx::AttributeArray &aa = *shape.GetAttributeArray();
    const ion::gfx::Attribute &pattr = aa.GetAttribute(0);
    ASSERT(pattr.Is<BOE>());
    pbh_.reset(new ConstBufferHelper_(pattr.GetValue<BOE>()));

    // Access tex coords if requested.
    if (need_tex_coords) {
        ASSERT(aa.GetAttributeCount() >= 2U);
        ASSERT(aa.GetBufferAttributeCount() >= 2U);
        const ion::gfx::Attribute &tattr = aa.GetAttribute(1);
        ASSERT(tattr.Is<BOE>());
        tbh_.reset(new MutableBufferHelper_(tattr.GetValue<BOE>()));
        ASSERT(tbh_->GetCount() == pbh_->GetCount());
    }

    // Access normals if requested.
    if (need_normals) {
        ASSERT(aa.GetAttributeCount()       >= (need_tex_coords ? 3U : 2U));
        ASSERT(aa.GetBufferAttributeCount() >= (need_tex_coords ? 3U : 2U));
        const int norm_index = aa.GetAttributeCount() == 3U ? 2 : 1;
        const ion::gfx::Attribute &nattr = aa.GetAttribute(norm_index);
        ASSERT(nattr.Is<BOE>());
        nbh_.reset(new MutableBufferHelper_(nattr.GetValue<BOE>()));
        ASSERT(nbh_->GetCount() == pbh_->GetCount());
    }

    // Access indices.
    ibh_.reset(new IndexBufferHelper_(*shape.GetIndexBuffer()));
}

void ShapeHelper_::SetFaceNormals(const std::vector<Vector3f> &normals) {
    ASSERT(nbh_.get());
    ASSERT(normals.size() * 3U == ibh_->GetCount());

    const size_t ncount = normals.size();
    for (size_t i = 0; i < ncount; ++i) {
        const int i0 = ibh_->GetIndex(3 * i);
        const int i1 = ibh_->GetIndex(3 * i + 1);
        const int i2 = ibh_->GetIndex(3 * i + 2);
        nbh_->GetData<Vector3f>(i0) = normals[i];
        nbh_->GetData<Vector3f>(i1) = normals[i];
        nbh_->GetData<Vector3f>(i2) = normals[i];
    }
}

void ShapeHelper_::SetVertexNormals(const std::vector<Vector3f> &normals) {
    ASSERT(nbh_.get());
    ASSERT(normals.size() == pbh_->GetCount());

    for (size_t i = 0; i < normals.size(); ++i)
        nbh_->GetData<Vector3f>(i) = normals[i];
}

void ShapeHelper_::SetTextureCoords(const std::vector<Point2f> &tex_coords) {
    ASSERT(tbh_.get());
    ASSERT(tex_coords.size() == tbh_->GetCount());

    for (size_t i = 0; i < tex_coords.size(); ++i)
        tbh_->GetData<Point2f>(i) = tex_coords[i];
}

void ShapeHelper_::GenerateFaceNormals() {
    ASSERT(nbh_.get());

    // Copy the face normal into each vertex.
    const size_t count = ibh_->GetCount();
    for (size_t i = 0; i < count; i += 3) {
        const int i0 = ibh_->GetIndex(i);
        const int i1 = ibh_->GetIndex(i + 1);
        const int i2 = ibh_->GetIndex(i + 2);
        const Vector3f normal = ComputeNormal(pbh_->GetData<Point3f>(i0),
                                              pbh_->GetData<Point3f>(i1),
                                              pbh_->GetData<Point3f>(i2));
        nbh_->GetData<Vector3f>(i0) = normal;
        nbh_->GetData<Vector3f>(i1) = normal;
        nbh_->GetData<Vector3f>(i2) = normal;
    }
}

void ShapeHelper_::GenerateVertexNormals() {
    ASSERT(nbh_.get());

    // Zero out all normals in case there is anything there.
    const size_t ncount = nbh_->GetCount();
    for (size_t i = 0; i < ncount; ++i)
        nbh_->GetData<Vector3f>(i).Set(0, 0, 0);

    // Sum the normals.
    const size_t icount = ibh_->GetCount();
    for (size_t i = 0; i < icount; i += 3) {
        const int i0 = ibh_->GetIndex(i);
        const int i1 = ibh_->GetIndex(i + 1);
        const int i2 = ibh_->GetIndex(i + 2);
        const Vector3f normal = ComputeNormal(pbh_->GetData<Point3f>(i0),
                                              pbh_->GetData<Point3f>(i1),
                                              pbh_->GetData<Point3f>(i2));
        nbh_->GetData<Vector3f>(i0) += normal;
        nbh_->GetData<Vector3f>(i1) += normal;
        nbh_->GetData<Vector3f>(i2) += normal;
    }

    // Normalize all of the vertex normals.
    for (size_t i = 0; i < ncount; ++i)
        ion::math::Normalize(&nbh_->GetData<Vector3f>(i));
}

void ShapeHelper_::GenerateTexCoords(SG::TriMeshShape::TexCoordsType type) {
    ASSERT(tbh_.get());

    // Figure out which dimensions to use.
    int dim0 = -1, dim1 = -1;
    GetTextureDimensions_(type, dim0, dim1);

    // Compute min/max point value in each requested dimension.
    const Range2f bounds = GetPointBounds_(dim0, dim1);

    // Set texture coordinates.
    const Point2f  &min = bounds.GetMinPoint();
    const Vector2f size = bounds.GetSize();
    const size_t count = pbh_->GetCount();
    for (size_t i = 0; i < count; ++i) {
        const Point3f &pt = pbh_->GetData<Point3f>(i);
        Point2f       &tc = tbh_->GetData<Point2f>(i);
        tc.Set((pt[dim0] - min[0]) / size[0],
               (pt[dim1] - min[1]) / size[1]);
    }
}

void ShapeHelper_::GetTextureDimensions_(SG::TriMeshShape::TexCoordsType type,
                                         int &dim0, int &dim1) {
    switch (type) {
        using enum SG::TriMeshShape::TexCoordsType;  // Shorthand.
      case kTexCoordsXY: dim0 = 0; dim1 = 1; break;
      case kTexCoordsXZ: dim0 = 0; dim1 = 2; break;
      case kTexCoordsYX: dim0 = 1; dim1 = 0; break;
      case kTexCoordsYZ: dim0 = 1; dim1 = 2; break;
      case kTexCoordsZX: dim0 = 2; dim1 = 0; break;
      case kTexCoordsZY: dim0 = 2; dim1 = 1; break;
      default: ASSERT(false);  // LCOV_EXCL_LINE [cannot happen]
    }
}

Range2f ShapeHelper_::GetPointBounds_(int dim0, int dim1) const {
    Point2f min = std::numeric_limits<float>::max() * Point2f(1, 1);
    Point2f max = std::numeric_limits<float>::min() * Point2f(1, 1);
    const size_t count = pbh_->GetCount();
    for (size_t i = 0; i < count; ++i) {
        const Point3f &pt = pbh_->GetData<Point3f>(i);
        min[0] = std::min(min[0], pt[dim0]);
        min[1] = std::min(min[1], pt[dim1]);
        max[0] = std::max(max[0], pt[dim0]);
        max[1] = std::max(max[1], pt[dim1]);
    }
    return Range2f(min, max);
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

void TriMeshShape::SetFaceNormals(const std::vector<Vector3f> &normals,
                                  ion::gfx::Shape &shape) {
    ShapeHelper_ sh(shape, true, false);
    sh.SetFaceNormals(normals);
}

void TriMeshShape::SetVertexNormals(const std::vector<Vector3f> &normals,
                                    ion::gfx::Shape &shape) {
    ShapeHelper_ sh(shape, true, false);
    sh.SetVertexNormals(normals);
}

void TriMeshShape::SetTextureCoords(const std::vector<Point2f> &tex_coords,
                                    ion::gfx::Shape &shape) {
    ShapeHelper_ sh(shape, false, true);
    sh.SetTextureCoords(tex_coords);
}

void TriMeshShape::GenerateFaceNormals(ion::gfx::Shape &shape) {
    ShapeHelper_ sh(shape, true, false);
    sh.GenerateFaceNormals();
}

void TriMeshShape::GenerateVertexNormals(ion::gfx::Shape &shape) {
    ShapeHelper_ sh(shape, true, false);
    sh.GenerateVertexNormals();
}

void TriMeshShape::GenerateTexCoords(TexCoordsType type,
                                     ion::gfx::Shape &shape) {
    ShapeHelper_ sh(shape, false, true);
    sh.GenerateTexCoords(type);
}

void TriMeshShape::FillTriMesh(const ion::gfx::Shape &shape) {
    tri_mesh_ = IonShapeToTriMesh(shape);
    ProcessChange(Change::kGeometry, *this);
}

void TriMeshShape::InstallMesh(const TriMesh &mesh) {
    tri_mesh_ = mesh;
    ProcessChange(Change::kGeometry, *this);
}

void TriMeshShape::CopyContentsFrom(const Parser::Object &from, bool is_deep) {
    Shape::CopyContentsFrom(from, is_deep);
    const TriMeshShape &from_tms = static_cast<const TriMeshShape &>(from);
    tri_mesh_ = from_tms.tri_mesh_;
}

}  // namespace SG
