#pragma once

#include "Base/Memory.h"
#include "Math/SlicedMesh.h"
#include "Math/Bend.h"
#include "Math/Types.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(BentModel);

/// BentModel is a derived ConvertedModel class that represents a Model that
/// has been bent by some angle around an arbitrary axis through an
/// arbitrary point. The point and axis are specified in object coordinates of
/// the operand Model.
///
/// \ingroup Models
class BentModel : public ConvertedModel {
  public:
    /// Sets the bend parameters.
    void SetBend(const Bend &bend);

    /// Returns the current Bend.
    const Bend & GetBend() const { return bend_; }

    virtual bool CanSetComplexity() const { return true; }

  protected:
    BentModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

    virtual TriMesh ConvertMesh(const TriMesh &mesh) override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<Point3f>  center_;
    Parser::TField<Vector3f> axis_;
    Parser::TField<Anglef>   angle_;
    ///@}

    /// Bend used to create the model.
    Bend      bend_;

    /// Caches the operand Model mesh split into slices based on complexity and
    /// the current Bend axis.
    SlicedMesh sliced_mesh_;

    /// Complexity used to create #sliced_mesh_.
    float      sliced_complexity_ = -1;

    /// Axis used to create #sliced_mesh_.
    Vector3f   sliced_axis_{0, 0, 0};

    friend class Parser::Registry;
};
