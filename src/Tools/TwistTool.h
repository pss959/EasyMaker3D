//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <memory>

#include "Tools/SpinBasedTool.h"

/// TwistTool provides interactive twisting of all selected TwistedModel
/// instances. It is derived from SpinBasedTool, so it uses a SpinWidget to
/// interact with the Spin used for twisting.
///
/// \ingroup Tools
class TwistTool : public SpinBasedTool {
  protected:
    TwistTool() {}

    virtual bool CanAttach(const Selection &sel) const override;

    // Required SpinBasedTool functions:
    virtual Spin GetObjectSpinFromModel() const override;
    virtual ChangeSpinCommandPtr CreateChangeSpinCommand() const override;

    friend class Parser::Registry;
};
