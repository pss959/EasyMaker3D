//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Executors/ConvertExecutorBase.h"

/// ConvertMirrorExecutor executes the ConvertMirrorCommand.
///
/// \ingroup Executors
class ConvertMirrorExecutor : public ConvertExecutorBase {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ConvertMirrorCommand";
    }

  protected:
    virtual ConvertedModelPtr CreateConvertedModel(const Str &name) override;

    /// Redefines this to use the primary MirroredModel's plane for all
    /// secondary selections.
    virtual void InitConvertedModel(ConvertedModel &model,
                                    const SelPath &path,
                                    const SelPath &primary_path) override;
};
