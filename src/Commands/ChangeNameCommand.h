//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/SingleModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeNameCommand);

/// ChangeNameCommand is used to change the name of the currently selected
/// Model.
///
/// \ingroup Commands
class ChangeNameCommand : public SingleModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets the new name.
    void SetNewName(const Str &new_name);

    /// Returns the new name.
    const Str & GetNewName() const { return new_name_; }

  protected:
    ChangeNameCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str> new_name_;
    ///@}

    friend class Parser::Registry;
};
