#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ClippedModel);

/// ClippedModel is a derived ConvertedModel class that represents a Model that
/// has been clipped by a plane specified in object coordinates of the operand
/// Model. Clipping changes the size of the mesh, so the ClippedModel's center
/// typically moves as well. The GetCenterOffset() function can be used to deal
/// with this.
///
/// \ingroup Models
class ClippedModel : public ConvertedModel {
  public:
    /// Sets the Plane (specified in object coordinates of the operand Model)
    /// to clip to. The side the Plane normal points toward is clipped
    /// away. The default is the XZ plane (clipping the +Y side).
    void SetPlane(const Plane &plane);

    /// Returns the clipping Plane (in object coordinates of the operand
    /// Model).
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
