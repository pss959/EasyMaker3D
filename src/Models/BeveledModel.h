#pragma once

#include "Base/Memory.h"
#include "Math/Bevel.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(BeveledModel);

/// BeveledModel is a derived ConvertedModel class that represents a Model
/// whose edges have had a bevel or rounding operation applied to them.
///
/// Note that the BeveledModel applies the bevel profile to the scaled version
/// of the original Model's mesh, since that is what the user
/// expects. Therefore, the scale factors in the BeveledModel differ from those
/// in the original. This stores the difference so the two can be kept in sync.
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
    virtual TriMesh BuildMesh() override;

    /// Redefines this to also copy the original scale.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;

    /// Redefines this to also mark the mesh as stale if the original Model's
    /// scale changed.
    virtual bool ProcessChange(SG::Change change, const Object &obj) override;

    /// Overrides this to deal with the difference in scale.
    virtual void SyncTransformsFromOriginal(const Model &original) override;

    /// Overrides this to deal with the difference in scale.
    virtual void SyncTransformsToOriginal(Model &original) const override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::VField<Point2f> profile_points_;
    Parser::TField<float>   bevel_scale_;
    Parser::TField<Anglef>  max_angle_;
    ///@}

    /// Bevel used to create the model.
    Bevel bevel_;

    /// Original scale factors applied to the original Model's mesh before
    /// beveling.
    Vector3f original_scale_{1, 1, 1};

    friend class Parser::Registry;
};
