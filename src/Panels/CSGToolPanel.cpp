#include "Panels/CSGToolPanel.h"

#include "Panes/RadioButtonPane.h"
#include "Util/Enum.h"

void CSGToolPanel::SetOperation(CSGOperation operation) {
    operation_ = operation;
}

void CSGToolPanel::InitInterface() {
    const auto &root_pane = GetPane();

    // Set up radio buttons. Note that these must be in the same order as the
    // CSGOperation enum values.
    std::vector<RadioButtonPanePtr> buts;
    buts.push_back(root_pane->FindTypedSubPane<RadioButtonPane>("Union"));
    buts.push_back(root_pane->FindTypedSubPane<RadioButtonPane>("Intersection"));
    buts.push_back(root_pane->FindTypedSubPane<RadioButtonPane>("Difference"));
    RadioButtonPane::CreateGroup(buts, 0);

    // Detect changes to the operation.
    auto set_op = [&](size_t index){
        const CSGOperation new_operation =
            Util::EnumFromIndex<CSGOperation>(index);
        if (new_operation != operation_) {
            operation_ = new_operation;
            ReportChange("Operation", InteractionType::kImmediate);
        }
    };
    for (auto &but: buts)
        but->GetStateChanged().AddObserver(this, set_op);
}

void CSGToolPanel::UpdateInterface() {
    // Turn on the correct radio button.
    const Str op_name = Util::EnumToWord(operation_);
    GetPane()->FindTypedSubPane<RadioButtonPane>(op_name)->SetState(true);
}
