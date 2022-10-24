#pragma once

#include <string>

#include "Base/Memory.h"
#include "Panels/Panel.h"

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
    void SetMessage(const std::string &msg);

    /// Sets up a single response button with the given text.
    void SetSingleResponse(const std::string &text);

    /// Sets up two response buttons with the given text. The focus_first flag
    /// indicates whether the first or second button should be focused by
    /// default.
    void SetChoiceResponse(const std::string &text0, const std::string &text1,
                           bool focus_first);

    /// Redefines this to reset the size so the next invocation uses the proper
    /// size.
    virtual void Close(const std::string &result) override;

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
