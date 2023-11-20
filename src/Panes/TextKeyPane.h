#pragma once

#include "Panes/KeyPane.h"
#include "Util/Memory.h"

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
    virtual TextAction GetAction(Str &chars) const override;

    /// Defines this to change the text label based on the current shift state.
    virtual void ProcessShift(bool is_shifted) override;

  protected:
    TextKeyPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str> text_;
    Parser::TField<Str> shifted_text_;
    Parser::TField<Str> label_;
    ///@}

    /// Stores the TextPane inside the TextKeyPane.
    TextPanePtr text_pane_;

    /// Returns the text to insert when the key is pressed.
    Str GetText_() const;

    friend class Parser::Registry;
};
