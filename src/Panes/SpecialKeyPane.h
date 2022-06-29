#pragma once

#include <string>

#include "Base/Memory.h"
#include "Panes/KeyPane.h"

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
    virtual TextAction GetAction(std::string &chars) const override {
        return action_;
    }

  protected:
    SpecialKeyPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<TextAction> action_{"action"};
    Parser::TField<std::string>   icon_name_{"icon_name"};
    Parser::TField<std::string>   label_{"label"};
    ///@}

    friend class Parser::Registry;
};
