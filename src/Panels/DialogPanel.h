#pragma once

#include <string>

#include "Memory.h"
#include "Panels/Panel.h"

DECL_SHARED_PTR(ButtonPane);
DECL_SHARED_PTR(TextPane);
DECL_SHARED_PTR(DialogPanel);

namespace Parser { class Registry; }

/// DialogPanel is a derived Panel class that displays a message and buttons to
/// allow the user to react. This can display either one or two response
/// buttons with customizable text.
class DialogPanel : public Panel {
  public:
    /// Sets the message to display in the dialog.
    void SetMessage(const std::string &msg);

    /// Sets up a single response button with the given text.
    void SetSingleResponse(const std::string &text);

    /// Sets up two response buttons with the given text.
    void SetChoiceResponse(const std::string &text0, const std::string &text1);

  protected:
    DialogPanel() {}

    virtual void InitInterface() override;

  private:
    TextPanePtr   message_;
    ButtonPanePtr button0_;
    ButtonPanePtr button1_;
    TextPanePtr   text0_;
    TextPanePtr   text1_;

    friend class Parser::Registry;
};
