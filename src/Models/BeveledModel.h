#pragma once

#include "Base/Memory.h"
#include "Math/Bevel.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(BeveledModel);

/// BeveledModel is a derived ConvertedModel class that represents a Model
/// whose edges have had a bevel or rounding operation applied to them.
///
/// \ingroup Models
class BeveledModel : public ConvertedModel {
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
    virtual bool IsValid(std::string &details) override;
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
