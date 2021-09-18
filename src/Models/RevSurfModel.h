#pragma once

#include "Models/Model.h"

#include "Math/Profile.h"
#include "Math/Types.h"

//! RevSurfModel is a derived Model that represents a surface of revolution.
//!
//! \ingroup Models
class RevSurfModel : public Model {
  public:
    //! Creates and returns a default Profile for a RevSurfModel.
    static Profile CreateDefaultProfile() {
        Profile profile(Point2f(0, 1), Point2f(0, 0));
        profile.AddPoint(Point2f(.5f, .5f));
        return profile;
    }

    //! The default constructor installs a default profile and sets the sweep
    //! angle to 360 degrees.
    RevSurfModel() {
        profile_     = CreateDefaultProfile();
        sweep_angle_ = Anglef::FromDegrees(360);
    }

    //! Sets the Profile to use for the surface.
    void SetProfile(const Profile &profile);

    //! Returns the current profile.
    const Profile & GetProfile() const { return profile_; }

    //! Sets the sweep angle.
    void SetSweepAngle(const Anglef &angle);

    //! Returns the current sweep angle.
    const Anglef & GetSweepAngle() const { return sweep_angle_; }

    //! RevSurfModel responds to complexity.
    virtual bool CanSetComplexity() const override { return true; }

  protected:
    virtual TriMesh BuildMesh() override;

  private:
    //! Profile used for the surface.
    Profile profile_;

    //! Angle to sweep the profile around. This should be greater than 0 and
    // less than or equal to 360. If this is not 360, the surface will subtend
    // the given angle and be capped with flat polygons at the start and end.
    Anglef  sweep_angle_;
};
