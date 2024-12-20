//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/SingleModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeOrderCommand);

/// ChangeOrderCommand is used to change the ordering of Models. It can be used
/// for a top-level Model or a child of a ParentModel.
///
/// \ingroup Commands
class ChangeOrderCommand : public SingleModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets the direction - if true, the Model is moved to the previous
    /// position as opposed to the next position.
    void SetIsPrevious(bool is_previous) { is_previous_ = is_previous; }

    /// Returns true if the Model is to be moved to the previous position as
    /// opposed to the next position.
    bool IsPrevious() const { return is_previous_; }

  protected:
    ChangeOrderCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool> is_previous_;
    ///@}

    friend class Parser::Registry;
};
