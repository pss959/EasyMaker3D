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

    /// Sets an offset in local coordinates of the ClippedModel relative to the
    /// original Model. The offset is the zero vector by default. This can be
    /// used to compensate for a change in translation due to clipping.
    void SetOffset(const Vector3f &offset) { offset_ = offset; }

    /// Returns the offset last set by SetOffset(). This is the zero vector by
    /// default.
    const Vector3f & GetOffset() const { return offset_; }

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

    /// Extra offset added to the original Model's translation in local
    /// coordinates.
    Vector3f offset_{0, 0, 0};

    friend class Parser::Registry;
};
