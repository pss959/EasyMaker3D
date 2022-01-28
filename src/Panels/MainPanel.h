#pragma once

#include <functional>
#include <string>

#include "Panels/Panel.h"

/// MainPanel is derived from Panel and serves as an abstract base class
/// that provides some conveniences for derived Panel classes.
class MainPanel : public Panel {
  protected:
    /// Type of function that is invoked by DisplayMessage().
    typedef std::function<void(void)> MessageFunc;

    /// Type of function that is invoked by AskQuestion().
    typedef std::function<void(const std::string &)> QuestionFunc;

    MainPanel() {}

    /// Convenience that opens a DialogPanel to display the given message along
    /// with an "OK" button that invokes the given function (if not null).
    void DisplayMessage(const std::string &message,
                        const MessageFunc &func);

    /// Convenience that opens a DialogPanel to ask the given question and get
    /// a "Yes" or "No" result, which is passed to the given function.
    void AskQuestion(const std::string &question, const QuestionFunc &func);

  private:
    /// Saves the MessageFunc passed to DisplayMessage() so it can be invoked
    /// later.
    MessageFunc  message_func_;

    /// Saves the QuestionFunc passed to AskQuestion() so it can be invoked
    /// later.
    QuestionFunc question_func_;
};
