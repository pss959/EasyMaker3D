#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ClippedModel);

/// ClippedModel is a derived ConvertedModel class that represents a Model that
/// has been clipped by a plane specified in object coordinates.
///
/// Clipping changes the size of the mesh, so the ClippedModel's center has to
/// move as well. It maintains an offset vector (in local coordinates) that is
/// used to keep the ClippedModel in the same relative position as the original
/// mesh after clipping. Therefore, the translation in the ClippedModel differs
/// from that in the original.
///
/// \ingroup Models
class ClippedModel : public ConvertedModel {
  public:
    /// Sets the Plane (specified in object coordinates) to clip to. The side
    /// the Plane normal points toward is clipped away. The default is the XZ
    /// plane (clipping the +Y side).
    void SetPlane(const Plane &plane);

    /// Returns the clipping Plane (in object coordinates).
    const Plane & GetPlane() const { return plane_; }

    /// Whenever the ClippedModel's mesh is rebuilt, it is recentered to put
    /// the mesh at the origin in object coordinates. This returns the offset
    /// used to recenter the mesh (in object coordinates) so that the
    /// ClippedModel can be positioned correctly relative to the original
    /// Model.
    const Vector3f & GetMeshOffset() const {
        // Make sure the mesh is up to date first.
        GetMesh();
        return mesh_offset_;
    }

  protected:
    ClippedModel() {}
    virtual void AddFields() override;
    virtual TriMesh BuildMesh() override;

    /// Overrides this to deal with the offset vector.
    virtual void SyncTransformsFromOriginal(const Model &original) override;

    /// Overrides this to deal with the offset vector.
    virtual void SyncTransformsToOriginal(Model &original) const override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<Plane> plane_;
    ///@}

    /// Offset used to center the mesh in object coordinates.
    Vector3f mesh_offset_{0, 0, 0};

    friend class Parser::Registry;
};
