#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"
#include "Math/Profile.h"
#include "Math/Types.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeRevSurfCommand);

/// ChangeRevSurfCommand is used to change the profile or sweep angle in one or
/// more RevSurfModel instances.
///
/// \ingroup Commands
class ChangeRevSurfCommand : public MultiModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets the new Profile points from the given Profile.
    void SetProfile(const Profile &profile);

    /// Returns a Profile corresponding to the new points.
    Profile GetProfile() const;

    /// Sets the new sweep angle.
    void SetSweepAngle(const Anglef &sweep_angle);

    /// Returns the new sweep angle.
    const Anglef & GetSweepAngle() const { return sweep_angle_; }

  protected:
    ChangeRevSurfCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::VField<Point2f> profile_points_;
    Parser::TField<Anglef>  sweep_angle_;
    ///@}

    friend class Parser::Registry;
};
