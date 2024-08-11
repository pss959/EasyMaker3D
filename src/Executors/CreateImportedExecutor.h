//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Executors/ModelExecutorBase.h"

/// CreateImportedExecutor executes the CreateImportedModelCommand.
///
/// \ingroup Executors
class CreateImportedExecutor : public ModelExecutorBase {
  public:
    virtual Str GetCommandTypeName() const override {
        return "CreateImportedModelCommand";
    }

  protected:
    virtual ModelPtr CreateModel(Command &command) override;
};
