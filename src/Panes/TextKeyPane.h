#pragma once

#include <string>

#include "Base/Memory.h"
#include "Panes/KeyPane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(TextKeyPane);
DECL_SHARED_PTR(TextPane);

/// TextKeyPane is a derived KeyPane that inserts text characters when
/// pressed. There are unshifted and shifted versions of the text; both must be
/// specified. The key displays the text (with the current shift state) as a
/// label, unless the "label" field is not empty, in which case it is always
/// used.
///
/// \ingroup Panes
class TextKeyPane : public KeyPane {
  public:
    virtual TextAction GetAction(std::string &chars) const override;

    /// Defines this to change the text label based on the current shift state.
    virtual void ProcessShift(bool is_shifted) override;

  protected:
    TextKeyPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> text_;
    Parser::TField<std::string> shifted_text_;
    Parser::TField<std::string> label_;
    ///@}

    /// Stores the TextPane inside the TextKeyPane.
    TextPanePtr text_pane_;

    /// Returns the text to insert when the key is pressed.
    std::string GetText_() const;

    friend class Parser::Registry;
};
