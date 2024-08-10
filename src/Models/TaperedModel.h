#pragma once

#include "Math/Taper.h"
#include "Models/ConvertedModel.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(TaperedModel);

/// TaperedModel is a derived ConvertedModel class that represents a Model
/// that is tapered along one of the principal axes according to a taper
/// Profile.
///
/// \ingroup Models
class TaperedModel : public ConvertedModel {
  public:
    /// Sets the Taper to use.
    void SetTaper(const Taper &taper);

    /// Returns the current Taper.
    const Taper & GetTaper() const { return taper_; }

    /// Convenience that creates a valid TaperedModel Profile from the given
    /// vector of Profile points. If there are no points, this uses the default
    /// Taper profile.
    static Profile CreateProfile(const Profile::PointVec &points);

  protected:
    TaperedModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

    virtual TriMesh ConvertMesh(const TriMesh &mesh) override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::EnumField<Dim>  axis_;
    Parser::VField<Point2f> profile_points_;
    ///@}

    /// Taper used to create the model.
    Taper      taper_;

    /// Previous Taper. This is used to optimize rebuilding the mesh.
    Taper      prev_taper_;

    friend class Parser::Registry;
};
