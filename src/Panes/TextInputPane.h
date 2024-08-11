//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Panes/BoxPane.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(TextInputPane);

/// TextInputPane is a derived BoxPane that supports interactive editing of
/// displayed text.
///
/// \ingroup Panes
class TextInputPane : public BoxPane {
  public:
    /// Alias for a function that is used to determine whether the current
    /// input text is valid. The current text is supplied.
    using ValidationFunc = std::function<bool(const Str &)>;

    /// Sets a function that is used to determine whether the current text is
    /// valid. If this function is not null and returns false, the background
    /// of the TextInputPane is changed to indicate an error.
    void SetValidationFunc(const ValidationFunc &func);

    /// Sets the initial text to display.
    void SetInitialText(const Str &text);

    /// Returns the current text. This will be empty until the TextInputPane is
    /// fully set up.
    Str GetText() const;

    /// Returns true if the text is currently marked as valid.
    bool IsTextValid() const;

    virtual void UpdateForLayoutSize(const Vector2f &size) override;
    virtual IPaneInteractor * GetInteractor() override;

  protected:
    TextInputPane();

    virtual void AddFields() override;
    virtual void CreationDone() override;

  private:
    struct Range_;
    class  State_;
    class  Stack_;
    class  Impl_;
    std::unique_ptr<Impl_> impl_;

    /// \name Parsed Fields
    ///@{
    Parser::TField<Str> initial_text_;
    ///@}

    friend class Parser::Registry;
};
