//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Executors/ModelExecutorBase.h"

/// CreateRevSurfExecutor executes the CreateRevSurfModelCommand.
///
/// \ingroup Executors
class CreateRevSurfExecutor : public ModelExecutorBase {
  public:
    virtual Str GetCommandTypeName() const override {
        return "CreateRevSurfModelCommand";
    }

  protected:
    virtual ModelPtr CreateModel(Command &command) override;
};
