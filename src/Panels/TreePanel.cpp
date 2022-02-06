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

/// A TreePanel::SessionRow_ instance represents the row at the top of the
/// TreePanel that shows the session name and has a button to toggle
/// visibility.
class TreePanel::SessionRow_ {
  public:
    SessionRow_(const ContainerPanePtr &pane);

    /// Sets the session text.
    void SetSessionText(const std::string &text);

  private:
    SwitcherPanePtr vis_switcher_pane_;  ///< Show/hide buttons.
    TextPanePtr     session_text_pane_;  ///< Session string TextPane.
};

TreePanel::SessionRow_::SessionRow_(const ContainerPanePtr &pane) {
    vis_switcher_pane_ = pane->FindTypedPane<SwitcherPane>("VisSwitcher");
    session_text_pane_ = pane->FindTypedPane<TextPane>("SessionString");
}

void TreePanel::SessionRow_::SetSessionText(const std::string &text) {
    if (session_text_pane_->GetText() != text)
        session_text_pane_->SetText(text);
}
// ----------------------------------------------------------------------------
// TreePanel::ModelRow_ class definition.
// ----------------------------------------------------------------------------

/// A TreePanel::ModelRow_ instance represents one row of the tree view
/// displaying the name of a Model and allows the TreePanel to interact with
/// it.
class TreePanel::ModelRow_ {
  public:
    ModelRow_(const ContainerPanePtr &pane);

  private:
    SwitcherPanePtr vis_switcher_pane_;  ///< Show/hide buttons.
    SwitcherPanePtr exp_switcher_pane_;  ///< Expand/collapse buttons.
    PanePtr         spacer_pane_;        ///< Spacer to indent button_pane_.
    ButtonPanePtr   button_pane_;        ///< Model Name button.
    TextPanePtr     text_pane_;          ///< Model name TextPane in button.
};

TreePanel::ModelRow_::ModelRow_(const ContainerPanePtr &pane) {
    vis_switcher_pane_ = pane->FindTypedPane<SwitcherPane>("VisSwitcher");
    exp_switcher_pane_ = pane->FindTypedPane<SwitcherPane>("ExpSwitcher");
    spacer_pane_       = pane->FindPane("Spacer");
    button_pane_       = pane->FindTypedPane<ButtonPane>("ModelButton");
    text_pane_         = button_pane_->FindTypedPane<TextPane>("Text");
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
    typedef std::shared_ptr<TreePanel::ModelRow_>   ModelRowPtr_;
    typedef std::shared_ptr<TreePanel::SessionRow_> SessionRowPtr_;

    ScrollingPanePtr scrolling_pane_;

    SessionRowPtr_            session_row_;
    std::vector<ModelRowPtr_> model_rows_;
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
    ASSERT(session_row_);
    session_row_->SetSessionText(str);
}

void TreePanel::Impl_::InitInterface(ContainerPane &root_pane) {
    scrolling_pane_ = root_pane.FindTypedPane<ScrollingPane>("Scroller");

    // Set up the session row.
    auto session_row_pane = root_pane.FindTypedPane<ContainerPane>("SessionRow");
    session_row_.reset(new SessionRow_(session_row_pane));

    // ModelRow_ instances will be created and added later.
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
