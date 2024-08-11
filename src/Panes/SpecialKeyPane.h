//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Panes/KeyPane.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(SpecialKeyPane);

/// SpecialKeyPane is a derived KeyPane that defines a key that implements a
/// special action (other than inserting text). The action is defined by a
/// field.
///
/// The label on the key can be either an icon or text. The "icon_name" field
/// names the icon to display, and the "label" field specifies the text label
/// to display. Exactly one of these must be non-empty.
///
/// \ingroup Panes
class SpecialKeyPane : public KeyPane {
  public:
    /// Returns the action to perform.
    virtual TextAction GetAction(Str &chars) const override {
        return action_;
    }

  protected:
    SpecialKeyPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<TextAction> action_;
    Parser::TField<Str>           icon_name_;
    Parser::TField<Str>           label_;
    ///@}

    friend class Parser::Registry;
};
