//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Profile.h"
#include "Math/Types.h"
#include "Models/PrimitiveModel.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(RevSurfModel);

/// RevSurfModel is a derived Model that represents a surface of revolution.
///
/// \ingroup Models
class RevSurfModel : public PrimitiveModel {
  public:
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

    /// Convenience that creates a valid RevSurfModel Profile from the given
    /// vector of movable profile points.
    static Profile CreateProfile(const Profile::PointVec &points);

    /// Convenience that creates and returns a default Profile for a
    /// RevSurfModel.
    static Profile CreateDefaultProfile();

  protected:
    RevSurfModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
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

    friend class Parser::Registry;
};
