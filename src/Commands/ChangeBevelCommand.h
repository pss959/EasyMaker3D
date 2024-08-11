//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/MultiModelCommand.h"
#include "Math/Bevel.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeBevelCommand);

/// ChangeBevelCommand is used to change the Bevel data in one or more
/// BeveledModel instances.
///
/// \ingroup Commands
class ChangeBevelCommand : public MultiModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets all the fields from the given Bevel.
    void SetBevel(const Bevel &bevel);

    /// Returns a Bevel representing the new Bevel data in the command.
    Bevel GetBevel() const;

  protected:
    ChangeBevelCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::VField<Point2f> profile_points_;
    Parser::TField<float>   bevel_scale_;
    Parser::TField<Anglef>  max_angle_;
    ///@}

    friend class Parser::Registry;
};
