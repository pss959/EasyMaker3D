#pragma once

#include <string>

#include "Base/Memory.h"
#include "Panes/KeyPane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(TextKeyPane);

/// TextKeyPane is a derived KeyPane that inserts text characters when
/// pressed. There are unshifted and shifted versions of the text; both must be
/// specified. The key displays the text (with the current shift state) as a
/// label, unless the "label" field is not empty, in which case it is always
/// used.
///
/// \ingroup Panes
class TextKeyPane : public KeyPane {
  public:
    virtual Action GetAction() const override { return Action::kInsertChars; }

    /// Changes the text label based on the current shift state.
    void ProcessShift(bool is_shifted);

    /// Returns the text to insert when the key is pressed.
    std::string GetText(bool is_shifted) const;

  protected:
    TextKeyPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> text_{"text"};
    Parser::TField<std::string> shifted_text_{"shifted_text"};
    Parser::TField<std::string> label_{"label"};
    ///@}

    friend class Parser::Registry;
};
