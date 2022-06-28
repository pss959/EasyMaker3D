#pragma once

#include "Base/Memory.h"
#include "Panes/ButtonPane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(KeyPane);

/// KeyPane is a derived ButtonPane that is used exclusively in the
/// KeyboardPane for defining keys on the virtual keyboard. A KeyPane may have
/// an Icon displayed on the keycap or a text string (if there is no Icon). The
/// KeyPane defines text to return when the key is pressed when shifted and
/// not.
///
/// \ingroup Panes
class KeyPane : public ButtonPane {
  public:
    /// Returns the text to insert when the key is pressed.
    std::string GetText(bool is_shifted) {
        return is_shifted ? shifted_text_ : text_;
    }

  protected:
    KeyPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> text_{"text"};
    Parser::TField<std::string> shifted_text_{"shifted_text"};
    Parser::TField<std::string> icon_name_{"icon_name"};
    ///@}

    friend class Parser::Registry;
};
