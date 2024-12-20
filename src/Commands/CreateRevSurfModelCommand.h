//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/CreateModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CreateRevSurfModelCommand);

/// CreateRevSurfModelCommand is used to create a RevSurfModel.
///
/// \ingroup Commands
class CreateRevSurfModelCommand : public CreateModelCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    CreateRevSurfModelCommand() {}

  private:
    friend class Parser::Registry;
};
