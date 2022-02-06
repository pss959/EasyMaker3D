#include "Panels/TreePanel.h"

#include <vector>

#include "Panes/ButtonPane.h"
#include "Panes/ContainerPane.h"
#include "Panes/ScrollingPane.h"
#include "Panes/SwitcherPane.h"
#include "Panes/TextPane.h"

// ----------------------------------------------------------------------------
// TreePanel::Row_ class definition.
// ----------------------------------------------------------------------------

/// A TreePanel::Row_ instance represents one row of the tree view and allows
/// the TreePanel to interact with it. There is a special instance for the top
/// row, which shows the session information.
class TreePanel::Row_ {
  public:
    /// Creates an instance to represent a row. The ContainerPane representing
    /// the row is passed in.
    Row_(const ContainerPanePtr &row_pane);

  private:
    SwitcherPanePtr vis_switcher_pane_;  ///< Show/hide buttons.
    SwitcherPanePtr exp_switcher_pane_;  ///< Expand/collapse buttons.
    PanePtr         spacer_pane_;        ///< Spacer to indent button_pane_.
    ButtonPanePtr   button_pane_;        ///< Session or Model name button.
};

TreePanel::Row_::Row_(const ContainerPanePtr &row_pane) {
    vis_switcher_pane_ = row_pane->FindTypedPane<SwitcherPane>("VisSwitcher");
    exp_switcher_pane_ = row_pane->FindTypedPane<SwitcherPane>("ExpSwitcher");
    spacer_pane_       = row_pane->FindPane("Spacer");
    button_pane_       = row_pane->FindTypedPane<ButtonPane>("ModelButton");
}

// ----------------------------------------------------------------------------
// TreePanel::Impl_ class definition.
// ----------------------------------------------------------------------------

class TreePanel::Impl_ {
  public:
    void Reset();
    void SetSessionString(const std::string &str);

    void InitInterface(ContainerPane &root_pane);

  private:
    typedef std::shared_ptr<TreePanel::Row_> RowPtr_;

    ScrollingPanePtr scrolling_pane_;

    std::vector<RowPtr_> rows_;
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
    // XXXX
    //if (session_pane_->GetText() != str)
    //session_pane_->SetText(str);
}

void TreePanel::Impl_::InitInterface(ContainerPane &root_pane) {
    // scrolling_pane_ = root_pane.FindTypedPane<ScrollingPane>("Scroller");

    // Create a Row_ instance for the top (session) row.
    auto session_row_pane = root_pane.FindTypedPane<ContainerPane>("SessionRow");
    rows_.push_back(RowPtr_(new Row_(session_row_pane)));
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
