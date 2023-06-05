#pragma once

#include "Base/Memory.h"
#include "Math/SlicedMesh.h"
#include "Math/Spin.h"
#include "Math/Types.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(TwistedModel);

/// TwistedModel is a derived ConvertedModel class that represents a Model that
/// has been twisted by a Spin specified in object coordinates of the operand
/// Model.
///
/// \ingroup Models
class TwistedModel : public ConvertedModel {
  public:
    /// Sets the spin parameters.
    void SetSpin(const Spin &spin);

    /// Returns the current Spin.
    const Spin & GetSpin() const { return spin_; }

    virtual bool CanSetComplexity() const { return true; }

  protected:
    TwistedModel() {}
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

    /// Spin used to create the model.
    Spin       spin_;

    /// Caches the operand Model mesh split into slices based on complexity and
    /// the current Spin axis.
    SlicedMesh sliced_mesh_;

    /// Complexity used to create #sliced_mesh_.
    float      sliced_complexity_ = -1;

    /// Axis used to create #sliced_mesh_.
    Vector3f   sliced_axis_{0, 0, 0};

    friend class Parser::Registry;
};
