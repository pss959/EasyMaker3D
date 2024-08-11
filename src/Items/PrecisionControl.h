//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "SG/Node.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(IconWidget);
DECL_SHARED_PTR(PrecisionControl);
namespace SG { DECL_SHARED_PTR(TextNode); }

/// The PrecisionControl class manages two precision control buttons on the
/// back wall and the feedback text.
///
/// \ingroup Items
class PrecisionControl : public SG::Node {
  public:
    /// Returns a vector containing the IconWidget instances used to modify the
    /// current precision values.
    std::vector<IconWidgetPtr> GetIcons() const;

    /// Updates text when a change is made to the current precision.
    void Update(float linear_precision, float angular_precision);

  protected:
    PrecisionControl() {}
    virtual void CreationDone() override;

  private:
    SG::TextNodePtr text_;

    friend class Parser::Registry;
};
