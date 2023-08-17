#include "Panels/TestPanel.h"

#include "Panes/ButtonPane.h"
#include "Panes/ContainerPane.h"
#include "Util/URL.h"

void TestPanel::InitInterface() {
    // Find all ButtonPane instances and set them up to print that they were
    // clicked. This keeps the Panel class from asserting.
    FindButtonPanes_(GetPane());
}

void TestPanel::UpdateInterface() {
}

void TestPanel::FindButtonPanes_(const PanePtr &pane) {
    if (auto but_pane = std::dynamic_pointer_cast<ButtonPane>(pane)) {
        const Str &name = but_pane->GetName();
        auto report = [&](){ std::cerr << "== Clicked " << name << "\n"; };
        AddButtonFunc(but_pane->GetName(), report);
    }
    if (auto ctr_pane = std::dynamic_pointer_cast<ContainerPane>(pane)) {
        for (auto &sub_pane: ctr_pane->GetPanes())
            FindButtonPanes_(sub_pane);
    }
}
