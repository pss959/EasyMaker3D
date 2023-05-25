#pragma once

#include "Base/Memory.h"
#include "Enums/Dim.h"
#include "Math/Bend.h"
#include "Math/SlicedMesh.h"
#include "Math/Types.h"
#include "Models/ScaledConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(BentModel);

/// BentModel is a derived ConvertedModel class that represents a Model that
/// has been bent by some angle around an arbitrary axis through an arbitrary
/// center point with an optional offset proportional to the angle. The center,
/// axis, and pffset are specified in object coordinates of the operand Model.
///
/// Note that the BentModel applies a Bend to the scaled version of the operand
/// Model's mesh, since that is what the user expects.  Therefore, it is
/// derived from ScaledConvertedModel so that the scales are maintained
/// properly.
///
/// \ingroup Models
class BentModel : public ScaledConvertedModel {
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
    Parser::TField<float>    offset_;
    ///@}

    /// Bend used to create the model.
    Bend       bend_;

    /// Caches the operand Model mesh split into slices based on complexity and
    /// the current Bend axis.
    SlicedMesh sliced_mesh_;

    /// Complexity used to create #sliced_mesh_.
    float      sliced_complexity_ = -1;

    /// Axis used to create #sliced_mesh_.
    Vector3f   sliced_axis_{0, 0, 0};

    /// Returns the principal dimension to slice along for the given Bend axis
    /// applied to the given TriMesh.
    static Dim GetSliceDim_(const TriMesh &mesh, const Vector3f &bend_axis);

    friend class Parser::Registry;
};
