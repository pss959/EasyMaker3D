#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ClippedModel);

/// ClippedModel is a derived ConvertedModel class that represents a Model that
/// has been clipped by one or more planes specified in object coordinates.
///
/// Note that the ClippedModel maintains an offset vector (in local
/// coordinates) that can be used to keep the ClippedModel in the same relative
/// position as the original mesh after clipping. (Since clipping changes the
/// size of the mesh, its center has to move as well.) Therefore, the
/// translation in the ClippedModel differs from that in the original.
///
/// \ingroup Models
class ClippedModel : public ConvertedModel {
  public:
    /// Adds a Plane (specified in object coordinates) to clip to. The side the
    /// Plane normal points toward is clipped away.
    void AddPlane(const Plane &local_plane);

    /// Removes the last Plane added. This should not be called if there is
    /// only one Plane.
    void RemoveLastPlane();

    /// Returns the current clipping Planes. This may be an empty vector.
    const std::vector<Plane> & GetPlanes() const { return planes_; }

    /// Whenever the ClippedModel's mesh is rebuilt, it is recentered to put
    /// the mesh at the origin in object coordinates. This returns the offset
    /// used to recenter the mesh (in object coordinates).
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
    Parser::VField<Plane> planes_;
    ///@}

    /// Offset used to center the mesh in object coordinates.
    Vector3f mesh_offset_{0, 0, 0};

    friend class Parser::Registry;
};
