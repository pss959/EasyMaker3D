#pragma once

#include "Base/Memory.h"
#include "Math/Spin.h"
#include "Math/Types.h"
#include "Models/ScaledConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(SpinBasedModel);

/// SpinBasedModel is a derived ConvertedModel class that is used as a base
/// class for Models that use a Spin to define some operation on the operand
/// Model. The Spin is specified in object coordinates of the operand Model.
///
/// Note that a SpinBasedModel applies a Spin to the scaled version of the
/// operand Model's mesh, since that is what the user expects.  Therefore, it
/// is derived from ScaledConvertedModel so that the scales are maintained
/// properly.
///
/// \ingroup Models
class SpinBasedModel : public ScaledConvertedModel {
  public:
    /// Sets the spin parameters.
    void SetSpin(const Spin &spin);

    /// Returns the current Spin.
    Spin GetSpin() const;

    virtual bool CanSetComplexity() const { return true; }

  protected:
    SpinBasedModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<Point3f>  center_;
    Parser::TField<Vector3f> axis_;
    Parser::TField<Anglef>   angle_;
    Parser::TField<float>    offset_;
    ///@}

    friend class Parser::Registry;
};
