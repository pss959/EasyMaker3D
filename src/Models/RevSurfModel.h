#pragma once

#include "Models/Model.h"

#include "Math/Profile.h"
#include "Math/Types.h"

//! RevSurfModel is a derived Model that represents a surface of revolution.
//!
//! \ingroup Models
class RevSurfModel : public Model {
  public:
    virtual void AddFields() override;

    //! Redefines this to fix up the Profile if it was read in.
    virtual void SetFieldParsed(const Parser::Field &field) override;

    //! Creates and returns a default Profile for a RevSurfModel.
    static Profile CreateDefaultProfile() {
        Profile profile(Point2f(0, 1), Point2f(0, 0));
        profile.AddPoint(Point2f(.5f, .5f));
        return profile;
    }

    //! Sets the Profile to use for the surface.
    void SetProfile(const Profile &profile);

    //! Returns the current profile.
    const Profile & GetProfile() const { return profile_; }

    //! Sets the sweep angle. This should be greater than 0 and less than or
    //! equal to 360. If this is not 360, the surface will subtend the given
    //! angle and be capped with flat polygons at the start and end.
    void SetSweepAngle(const Anglef &angle);

    //! Returns the current sweep angle.
    const Anglef & GetSweepAngle() const { return sweep_angle_; }

    //! RevSurfModel responds to complexity.
    virtual bool CanSetComplexity() const override { return true; }

  protected:
    virtual TriMesh BuildMesh() override;

  private:
    //! \name Parsed fields.
    //!@{
    Parser::TField<Profile> profile_{"profile"};
    Parser::TField<Anglef>  sweep_angle_{"sweep_angle",
                                         Anglef::FromDegrees(360) };
    //!@}
};

typedef std::shared_ptr<RevSurfModel> RevSurfModelPtr;
