//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/CombineCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CombineHullModelCommand);

/// CombineHullModelCommand is used to create a HullModel from one or more
/// operand Models.
///
/// \ingroup Commands
class CombineHullModelCommand : public CombineCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    CombineHullModelCommand() {}

  private:
    friend class Parser::Registry;
};
