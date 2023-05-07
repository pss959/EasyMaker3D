#pragma once

#include "Base/Memory.h"
#include "Math/Bevel.h"  // XXXX
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(TaperedModel);

/// TaperedModel is a derived ConvertedModel class that represents a Model
/// whose edges have had a taper or rounding operation applied to them.
///
/// Note that the TaperedModel applies the taper profile to the scaled version
/// of the operand Model's mesh, since that is what the user expects.
/// Therefore, the scale factors in the TaperedModel differ from those in the
/// operand. This stores the difference so the two can be kept in sync.
///
/// \ingroup Models
class TaperedModel : public ConvertedModel {
  public:
    /// Sets the Taper to use.
    void SetTaper(const Bevel &taper);

    /// Returns the current Taper.
    const Bevel & GetTaper() const { return taper_; }

    /// Convenience that creates a valid TaperedModel Profile from the given
    /// vector of movable profile points.
    static Profile CreateProfile(const Profile::PointVec &points);

  protected:
    TaperedModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

    /// Redefines this to also copy the operand scale.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;

    /// Overrides this to deal with the difference in scale.
    virtual void SyncTransformsFromOperand(const Model &operand) override;

    /// Overrides this to deal with the difference in scale.
    virtual void SyncTransformsToOperand(Model &operand) const override;

    virtual TriMesh ConvertMesh(const TriMesh &mesh) override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::VField<Point2f> profile_points_;
    Parser::TField<float>   taper_scale_;
    Parser::TField<Anglef>  max_angle_;
    ///@}

    /// Taper used to create the model.
    Bevel taper_;

    /// Scale factors applied to the operand Model's mesh before tapering.
    Vector3f operand_scale_{1, 1, 1};

    friend class Parser::Registry;
};
