//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/ConvertCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ConvertTwistCommand);

/// ConvertTwistCommand is used to convert each selected Model that is not a
/// TwistedModel to a TwistedModel.
///
/// \ingroup Commands
class ConvertTwistCommand : public ConvertCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    ConvertTwistCommand() {}

  private:
    friend class Parser::Registry;
};
