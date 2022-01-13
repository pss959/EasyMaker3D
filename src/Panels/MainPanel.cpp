#include "Panels/MainPanel.h"

#include "Panels/DialogPanel.h"

void MainPanel::DisplayMessage(const std::string &message,
                               const std::function<void(void)> &func) {
    auto init = [&](Panel &p){
        ASSERT(p.GetTypeName() == "DialogPanel");
        DialogPanel &dp = static_cast<DialogPanel &>(p);
        dp.SetMessage(message);
        dp.SetSingleResponse("OK");
    };
    auto result = [&](Panel &, const std::string &){ if (func) func(); };
    GetContext().panel_helper->Replace("DialogPanel", init, result);
}

void MainPanel::AskQuestion(
    const std::string &question,
    const std::function<void(const std::string &)> &func) {
    ASSERT(func);
    auto init = [&](Panel &p){
        ASSERT(p.GetTypeName() == "DialogPanel");
        DialogPanel &dp = static_cast<DialogPanel &>(p);
        dp.SetMessage(question);
        dp.SetChoiceResponse("No", "Yes");
    };
    auto result = [&](Panel &, const std::string &res){ func(res); };
    GetContext().panel_helper->Replace("DialogPanel", init, result);
}

