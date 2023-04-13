#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(MirroredModel);

/// MirroredModel is a derived ConvertedModel class that represents a Model
/// that has been mirrored across a plane specified in object coordinates of
/// the operand Model.
///
/// \ingroup Models
class MirroredModel : public ConvertedModel {
  public:
    /// Sets the Plane (specified in object coordinates of the operand Model)
    /// to mirror across.
    void SetPlane(const Plane &plane);

    /// Returns the mirroring Plane (in object coordinates of the operand
    /// Model).
    const Plane & GetPlane() const { return plane_; }

  protected:
    MirroredModel() {}
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
