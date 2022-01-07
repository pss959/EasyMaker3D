#pragma once

#include <string>

#include "Panels/Panel.h"
#include "Panes/ButtonPane.h"
#include "Panes/TextPane.h"

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

    /// Returns the string text from the button that the user selected to close
    /// the DialogPanel. This will be empty until after the DialogPanel is
    /// closed by a choice.
    const std::string & GetResponseString() const { return response_string_; }

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
    std::string   response_string_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<DialogPanel> DialogPanelPtr;
