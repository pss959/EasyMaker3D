//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/MultiModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

class EdgeTarget;
DECL_SHARED_PTR(LinearLayoutCommand);

/// LinearLayoutCommand is used to lay out the currently selected Models
/// linearly with equal spacing.
///
/// \ingroup Commands
class LinearLayoutCommand : public MultiModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets the offset in the command from an EdgeTarget instance.
    void SetFromTarget(const EdgeTarget &target);

    /// Returns the offset vector.
    const Vector3f & GetOffset() const { return offset_; }

  protected:
    LinearLayoutCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector3f> offset_;
    ///@}

    friend class Parser::Registry;
};
