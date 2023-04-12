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
/// used to translate the ClippedModel so it stays in the same relative
/// position as the original mesh after clipping. (This makes the translation
/// in the ClippedModel different from that in the original.)
///
/// The clipping plane is stored without considering the offset. The SetPlane()
/// and GetPlane() functions apply and remove the effects of the offset,
/// respectively.
///
/// \ingroup Models
class ClippedModel : public ConvertedModel {
  public:
    /// Sets the Plane (specified in object coordinates) to clip to. The side
    /// the Plane normal points toward is clipped away. The default is the XZ
    /// plane (clipping the +Y side). This assumes the given plane includes the
    /// effects of the mesh offset.
    void SetPlane(const Plane &plane);

    /// Returns the clipping Plane in object coordinates without accounting for
    /// the mesh offset. This is the Plane read in or the last Plane passed to
    /// SetPlane().
    const Plane & GetPlane() const { return plane_; }

    /// Returns the clipping Plane in object coordinates after accounting for
    /// the mesh offset.
    const Plane & GetOffsetPlane() const { return offset_plane_; }

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
    /// Plane in object coordinates without compensating for the mesh offset.
    Parser::TField<Plane> plane_;
    ///@}

    /// Offset used to center the mesh in object coordinates. This is updated
    /// whenever the mesh is rebuilt.
    Vector3f mesh_offset_{0, 0, 0};

    /// Plane in object coordinates that compensates for the mesh offset.
    Plane offset_plane_;

    /// Updates the offset vector, offset plane, and translation to compensate
    /// for a change in mesh centering.
    void UpdateMeshOffset_(const TriMesh &mesh);

    friend class Parser::Registry;
};
