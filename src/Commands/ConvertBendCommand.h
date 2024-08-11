//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/ConvertCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ConvertBendCommand);

/// ConvertBendCommand is used to convert each selected Model that is not a
/// BentModel to a BentModel.
///
/// \ingroup Commands
class ConvertBendCommand : public ConvertCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    ConvertBendCommand() {}

  private:
    friend class Parser::Registry;
};
