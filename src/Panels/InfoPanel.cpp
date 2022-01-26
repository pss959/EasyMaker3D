#include "Panels/InfoPanel.h"

#include "Models/Model.h"
#include "Targets/EdgeTarget.h"
#include "Targets/PointTarget.h"
#include "Util/Assert.h"

void InfoPanel::Reset() {
    text_.clear();
    std::cerr << "XXXX Reset text_ = " << text_ << "\n";
}

void InfoPanel::AddModel(const Model &model) {
    std::string s;
    if (! text_.empty())
        text_ += "\n";
    text_ += model.GetDesc() + "\n"; // XXXX
    std::cerr << "XXXX AddModel text_ = " << text_ << "\n";
}

void InfoPanel::AddPointTarget(const PointTarget &pt) {
    // XXXX
}

void InfoPanel::AddEdgeTarget(const EdgeTarget &et) {
    // XXXX
}

void InfoPanel::InitInterface() {
    text_pane_ = GetPane()->FindTypedPane<TextPane>("Text");

    AddButtonFunc("Done", [this](){ Close("Done");     });
}

void InfoPanel::UpdateInterface() {
    ASSERT(text_pane_);
    std::cerr << "XXXX UpdateInterface text_ = " << text_ << "\n";
    std::cerr << "XXXX UpdateInterface text_pane_ = "
              << text_pane_->GetDesc() << "\n";
    if (! text_.empty() && text_pane_->GetText() != text_)
        text_pane_->SetText(text_);

    SetFocus("Done");
}
