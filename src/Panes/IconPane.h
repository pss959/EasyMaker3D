//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Panes/LeafPane.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(IconPane);

/// IconPane is a derived LeafPane that displays an icon from the icon atlas.
///
/// \ingroup Panes
class IconPane : public LeafPane {
  public:
    /// Sets the name of the icon to display.
    void SetIconName(const Str &name);

    /// Returns the name of the icon to display.
    const Str & GetIconName() const { return icon_name_; }

    virtual Str ToString(bool is_brief) const override;

  protected:
    IconPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str> icon_name_;
    ///@}

    friend class Parser::Registry;
};
