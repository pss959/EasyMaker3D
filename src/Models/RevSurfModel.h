#pragma once

#include "Base/Memory.h"
#include "Math/Profile.h"
#include "Math/Types.h"
#include "Models/PrimitiveModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(RevSurfModel);

/// RevSurfModel is a derived Model that represents a surface of revolution.
///
/// \ingroup Models
class RevSurfModel : public PrimitiveModel {
  public:
    /// Creates and returns a default Profile for a RevSurfModel.
    static Profile CreateDefaultProfile() {
        Profile profile(Point2f(0, 1), Point2f(0, 0));
        profile.AddPoint(Point2f(.5f, .5f));
        return profile;
    }

    /// Sets the Profile to use for the surface.
    void SetProfile(const Profile &profile);

    /// Returns the current profile.
    const Profile & GetProfile() const { return profile_; }

    /// Sets the sweep angle. This should be greater than 0 and less than or
    /// equal to 360. If this is not 360, the surface will subtend the given
    /// angle and be capped with flat polygons at the start and end.
    void SetSweepAngle(const Anglef &angle);

    /// Returns the current sweep angle.
    const Anglef & GetSweepAngle() const { return sweep_angle_; }

    /// RevSurfModel responds to complexity.
    virtual bool CanSetComplexity() const override { return true; }

    /// Returns the offset vector used to center the RevSurfModel's mesh if it
    /// is not a full 360-degree sweep. This will be the zero vector for a full
    /// sweep.
    const Vector3f & GetCenterOffset() const { return center_offset_; }

  protected:
    RevSurfModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;
    virtual TriMesh BuildMesh() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::VField<Point2f> profile_points_;
    Parser::TField<Anglef>  sweep_angle_;
    ///@}

    /// Stores the current Profile.
    Profile profile_{ CreateDefaultProfile() };

    /// Offset added to the mesh in object coordinates to center it, if not a
    /// full 360-degree sweep.
    Vector3f center_offset_{0, 0, 0};

    friend class Parser::Registry;
};
