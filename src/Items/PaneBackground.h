//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"
#include "SG/Node.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(PaneBackground);

namespace Parser { class Registry; }

/// PaneBackground is used to add a colored background to any Pane.
///
/// \ingroup Items
class PaneBackground : public SG::Node {
  public:
    virtual void PostSetUpIon() override;

    const Color & GetColor() const { return color_; }

  protected:
    PaneBackground();

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Color> color_;
    ///@}

    friend class Parser::Registry;
};
