#include "Panels/MainPanel.h"

#include "Panels/DialogPanel.h"

void MainPanel::DisplayMessage(const std::string &message,
                               const MessageFunc &func) {
    auto init = [&](Panel &p){
        ASSERT(p.GetTypeName() == "DialogPanel");
        DialogPanel &dp = static_cast<DialogPanel &>(p);
        dp.SetMessage(message);
        dp.SetSingleResponse("OK");
    };

    // Save the function so it is around when the DialogPanel finishes.
    message_func_ = func;

    auto result = [&](Panel &, const std::string &){
        if (message_func_) {
            message_func_();
            message_func_ = nullptr;
        }
    };
    GetContext().panel_helper->Replace("DialogPanel", init, result);
}

void MainPanel::AskQuestion(const std::string &question,
                            const QuestionFunc &func) {
    ASSERT(func);
    auto init = [&](Panel &p){
        ASSERT(p.GetTypeName() == "DialogPanel");
        DialogPanel &dp = static_cast<DialogPanel &>(p);
        dp.SetMessage(question);
        dp.SetChoiceResponse("No", "Yes");
    };

    // Save the function so it is around when the DialogPanel finishes.
    question_func_ = func;

    auto result = [&](Panel &, const std::string &res){
        question_func_(res);
        question_func_ = nullptr;
    };
    GetContext().panel_helper->Replace("DialogPanel", init, result);
}
