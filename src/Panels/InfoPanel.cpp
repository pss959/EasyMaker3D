#include "Panels/InfoPanel.h"

#include "Models/Model.h"
#include "Targets/EdgeTarget.h"
#include "Targets/PointTarget.h"
#include "Util/Assert.h"
#include "Util/String.h"

void InfoPanel::CreationDone() {
    Panel::CreationDone();

    auto &root_pane = GetPane();
    contents_pane_ = root_pane->FindTypedPane<ContainerPane>("Contents");
    text_pane_     = root_pane->FindTypedPane<TextPane>("InfoText");
}

void InfoPanel::SetInfo(const Info &info) {
    ASSERT(! info.models.empty() || info.point_target || info.edge_target);

    // Create a vector containing a clone of the TextPane for each line of
    // information.
    std::vector<PanePtr> panes;

    for (const auto &model: info.models) {
        ASSERT(model);
        panes.push_back(CreateTextPane_("Line" + Util::ToString(panes.size()),
                                        model->GetDesc()));
    }

    contents_pane_->ReplacePanes(panes);
}

PanePtr InfoPanel::CreateTextPane_(const std::string &name,
                                   const std::string &text) {
    auto pane = text_pane_->CloneTyped<TextPane>(true, name);
    pane->SetText(text);
    pane->SetEnabled(SG::Node::Flag::kTraversal, true);
    return pane;
}

void InfoPanel::InitInterface() {
    AddButtonFunc("Done", [this](){ Close("Done");     });
}

void InfoPanel::UpdateInterface() {
    SetFocus("Done");
}
