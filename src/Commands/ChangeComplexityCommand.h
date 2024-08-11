//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/MultiModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeComplexityCommand);

/// ChangeComplexityCommand is used to change the complexity of the currently
/// selected Models.
///
/// \ingroup Commands
class ChangeComplexityCommand : public MultiModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets the new complexity.
    void SetNewComplexity(float new_complexity) {
        new_complexity_ = new_complexity;
    }

    /// Returns the new complexity.
    float GetNewComplexity() const { return new_complexity_; }

  protected:
    ChangeComplexityCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<float> new_complexity_;
    ///@}

    friend class Parser::Registry;
};
