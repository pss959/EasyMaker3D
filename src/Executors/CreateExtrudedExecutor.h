//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Executors/ModelExecutorBase.h"

/// CreateExtrudedExecutor executes the CreateExtrudedModelCommand.
///
/// \ingroup Executors
class CreateExtrudedExecutor : public ModelExecutorBase {
  public:
    virtual Str GetCommandTypeName() const override {
        return "CreateExtrudedModelCommand";
    }

  protected:
    virtual ModelPtr CreateModel(Command &command) override;
};
