//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/CreateModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CreateExtrudedModelCommand);

/// CreateExtrudedModelCommand is used to create a ExtrudedModel.
///
/// \ingroup Commands
class CreateExtrudedModelCommand : public CreateModelCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    CreateExtrudedModelCommand() {}

  private:
    friend class Parser::Registry;
};
