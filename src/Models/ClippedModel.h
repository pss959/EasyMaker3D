﻿#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ClippedModel);

/// ClippedModel is a derived ConvertedModel class that represents a Model that
/// has been clipped by a Plane: the half-space in the direction of the Plane's
/// normal is clipped away.
///
/// The clipping plane is specified in the object coordinates of the
/// ClippedModel. That is, the plane is applied to the operand Model's mesh
/// before it has been scaled, rotated, and translated.
///
/// Clipping changes the size of the mesh, so the ClippedModel's center
/// typically moves as well. The GetCenterOffset() function can be used to get
/// the offset vector used to reposition the ClippedModel. Note that the Plane
/// should \em not include this offset; it should be relative to the center of
/// the unclipped object. For example, a Plane through the center of the
/// unclipped object should have a distance of 0.
///
/// \ingroup Models
class ClippedModel : public ConvertedModel {
  public:
    /// Sets the Plane (in object coordinates without the centering offset) to
    /// clip to. The half-space in the direction of the Plane normal points
    /// toward is clipped away. The default is the XZ plane (clipping the +Y
    /// side).
    void SetPlane(const Plane &plane);

    /// Returns the clipping Plane (in object coordinates without the centering
    /// offset).
    const Plane & GetPlane() const { return plane_; }

  protected:
    ClippedModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

    virtual TriMesh ConvertMesh(const TriMesh &mesh) override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<Plane> plane_;
    ///@}

    friend class Parser::Registry;
};
