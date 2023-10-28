#pragma once

#include <string>

#include "Panels/Panel.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(ButtonPane);
DECL_SHARED_PTR(TextPane);
DECL_SHARED_PTR(DialogPanel);

namespace Parser { class Registry; }

/// DialogPanel is a derived Panel class that displays a message and buttons to
/// allow the user to react. This can display either one or two response
/// buttons with customizable text.
///
/// \ingroup Panels
class DialogPanel : public Panel {
  public:
    /// Sets the message to display in the dialog.
    void SetMessage(const Str &msg);

    /// Sets up a single response button with the given text.
    void SetSingleResponse(const Str &text);

    /// Sets up two response buttons with the given text. The focus_first flag
    /// indicates whether the first or second button should be focused by
    /// default.
    void SetChoiceResponse(const Str &text0, const Str &text1,
                           bool focus_first);

  protected:
    DialogPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    TextPanePtr   message_;
    ButtonPanePtr button0_;
    ButtonPanePtr button1_;
    TextPanePtr   text0_;
    TextPanePtr   text1_;

    friend class Parser::Registry;
};
