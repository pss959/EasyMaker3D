#pragma once

#include "Base/Memory.h"
#include "Panes/BoxPane.h"

class VirtualKeyboard;

namespace Parser { class Registry; }

DECL_SHARED_PTR(TextInputPane);

/// TextInputPane is a derived BoxPane that supports interactive editing of
/// displayed text.
///
/// \ingroup Panes
class TextInputPane : public BoxPane {
  public:
    /// Typedef for a function that is used to determine whether the current
    /// input text is valid. The current text is supplied.
    typedef std::function<bool(const std::string &)> ValidationFunc;

    /// Sets a function that is used to determine whether the current text is
    /// valid. If this function is not null and returns false, the background
    /// of the TextInputPane is changed to indicate an error.
    void SetValidationFunc(const ValidationFunc &func);

    /// Sets the initial text to display.
    void SetInitialText(const std::string &text);

    /// Returns the current text. This will be empty until the TextInputPane is
    /// fully set up.
    std::string GetText() const;

    virtual void SetLayoutSize(const Vector2f &size) override;
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
    Parser::TField<std::string> initial_text_;
    ///@}

    friend class Parser::Registry;
};
