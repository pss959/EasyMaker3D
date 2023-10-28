#pragma once

#include "Math/Bevel.h"
#include "Models/ScaledConvertedModel.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(BeveledModel);

/// BeveledModel is a derived ScaledConvertedModel class that represents a
/// Model whose edges have had a bevel or rounding operation applied to them.
///
/// Note that the BeveledModel applies the bevel profile to the scaled version
/// of the operand Model's mesh, since that is what the user expects.
/// Therefore, it is derived from ScaledConvertedModel so that the scales are
/// maintained properly.
///
/// \ingroup Models
class BeveledModel : public ScaledConvertedModel {
  public:
    /// Sets the Bevel to use.
    void SetBevel(const Bevel &bevel);

    /// Returns the current Bevel.
    const Bevel & GetBevel() const { return bevel_; }

    /// Convenience that creates a valid BeveledModel Profile from the given
    /// vector of movable profile points.
    static Profile CreateProfile(const Profile::PointVec &points);

  protected:
    BeveledModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

    virtual TriMesh ConvertMesh(const TriMesh &mesh) override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::VField<Point2f> profile_points_;
    Parser::TField<float>   bevel_scale_;
    Parser::TField<Anglef>  max_angle_;
    ///@}

    /// Bevel used to create the model.
    Bevel bevel_;

    friend class Parser::Registry;
};
