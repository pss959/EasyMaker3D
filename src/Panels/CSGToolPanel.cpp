#include "Panels/CSGToolPanel.h"

#include "Panes/RadioButtonPane.h"


void CSGToolPanel::SetOperation(CSGOperation operation) {
    operation_ = operation;
}

void CSGToolPanel::InitInterface() {
    const auto &root_pane = GetPane();

    // Set up radio buttons. Note that these must be in the same order as the
    // CSGOperation enum values.
    std::vector<RadioButtonPanePtr> buts;
    buts.push_back(root_pane->FindTypedPane<RadioButtonPane>("Union"));
    buts.push_back(root_pane->FindTypedPane<RadioButtonPane>("Intersection"));
    buts.push_back(root_pane->FindTypedPane<RadioButtonPane>("Difference"));
    RadioButtonPane::CreateGroup(buts, 0);

    // Detect changes to the operation.
    auto set_op = [&](size_t index){
        operation_ = static_cast<CSGOperation>(index);
        ReportChange("Operation", InteractionType::kImmediate);
    };
    for (auto &but: buts)
        but->GetStateChanged().AddObserver(this, set_op);
}

void CSGToolPanel::UpdateInterface() {
    // Turn on the correct radio button.
    const std::string op_name = Util::EnumToWord(operation_);
    GetPane()->FindTypedPane<RadioButtonPane>(op_name)->SetState(true);
}
