//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/ChangeSpinCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeBendCommand);

/// ChangeBendCommand is used to change the Spin applied one or more BentModel
/// instances. The Spin is always interpreted in object coordinates of each
/// model.
///
/// \ingroup Commands
class ChangeBendCommand : public ChangeSpinCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    ChangeBendCommand() {}

    friend class Parser::Registry;
};
