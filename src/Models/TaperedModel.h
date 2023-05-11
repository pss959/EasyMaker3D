#pragma once

#include "Base/Memory.h"
#include "Math/SlicedMesh.h"
#include "Math/Taper.h"
#include "Models/ConvertedModel.h"

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
    /// vector of Profile points.
    static Profile CreateProfile(const Profile::PointVec &points);

  protected:
    TaperedModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

    virtual TriMesh ConvertMesh(const TriMesh &mesh) override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::EnumField<Axis> axis_;
    Parser::VField<Point2f> profile_points_;
    ///@}

    /// Taper used to create the model.
    Taper      taper_;

    /// Previous Taper. This is used to optimize rebuilding the mesh.
    Taper      prev_taper_;

    /// Caches the operand Model mesh split into slices based on the current
    /// Taper profile.
    SlicedMesh sliced_mesh_;

    /// Returns the index of the 1 profile point that changed from #prev_taper_
    /// to #taper_. This returns -1 if the axis changed, the number of points
    /// changed, or if more than one point changed.
    int GetChangedProfileIndex_() const;

    friend class Parser::Registry;
};
