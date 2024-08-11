//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <memory>

#include "Tools/SpinBasedTool.h"

/// BendTool provides interactive bending of all selected BentModel
/// instances. It is derived from SpinBasedTool, so it uses a SpinWidget to
/// interact with the Spin used for bending.
///
/// \ingroup Tools
class BendTool : public SpinBasedTool {
  protected:
    BendTool() {}

    virtual bool CanAttach(const Selection &sel) const override;

    // Required SpinBasedTool functions:
    virtual Spin GetObjectSpinFromModel() const override;
    virtual ChangeSpinCommandPtr CreateChangeSpinCommand() const override;

    friend class Parser::Registry;
};
