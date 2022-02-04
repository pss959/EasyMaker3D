#include "Panels/TreePanel.h"

#include "Panes/ContainerPane.h"
#include "Panes/TextPane.h"

// ----------------------------------------------------------------------------
// TreePanel::Impl_ class definition.
// ----------------------------------------------------------------------------

class TreePanel::Impl_ {
  public:
    void Reset();
    void SetSessionString(const std::string &str);

    void InitInterface(ContainerPane &root_pane);

  private:
    TextPanePtr session_pane_;
};

// ----------------------------------------------------------------------------
// TreePanel::Impl_ functions.
// ----------------------------------------------------------------------------

void TreePanel::Impl_::Reset() {
    // XXXX
    //if (_treeView != null) {
    //_treeView.Clear();
    //_treeView.SetSessionName("<Untitled>");
    //_treeView.UpdateModels();
    //}
}

void TreePanel::Impl_::SetSessionString(const std::string &str) {
    if (session_pane_->GetText() != str)
        session_pane_->SetText(str);
}

void TreePanel::Impl_::InitInterface(ContainerPane &root_pane) {
    session_pane_ = root_pane.FindTypedPane<TextPane>("Session");
}

// ----------------------------------------------------------------------------
// TreePanel functions.
// ----------------------------------------------------------------------------

TreePanel::TreePanel() : impl_(new Impl_) {
    Reset();
}

void TreePanel::Reset() {
    impl_->Reset();
}

void TreePanel::SetSessionString(const std::string &str) {
    impl_->SetSessionString(str);
}

void TreePanel::InitInterface() {
    impl_->InitInterface(*GetPane());
}
