//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/MultiModelCommand.h"
#include "Math/Profile.h"
#include "Math/Types.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeExtrudedCommand);

/// ChangeExtrudedCommand is used to change the profile or sweep angle in one or
/// more ExtrudedModel instances.
///
/// \ingroup Commands
class ChangeExtrudedCommand : public MultiModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets the new Profile points from the given Profile.
    void SetProfile(const Profile &profile);

    /// Returns a Profile corresponding to the new points.
    Profile GetProfile() const;

  protected:
    ChangeExtrudedCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::VField<Point2f> profile_points_;
    ///@}

    friend class Parser::Registry;
};
