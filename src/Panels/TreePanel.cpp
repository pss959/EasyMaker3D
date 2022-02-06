#include "Panels/TreePanel.h"

#include <unordered_map>
#include <vector>

#include "Panes/ButtonPane.h"
#include "Panes/ContainerPane.h"
#include "Panes/ScrollingPane.h"
#include "Panes/SwitcherPane.h"
#include "Panes/TextPane.h"
#include "Util/Assert.h"

// ----------------------------------------------------------------------------
// TreePanel::Row_ class definition.
// ----------------------------------------------------------------------------

/// A TreePanel::SessionRow_ instance represents the row at the top of the
/// TreePanel that shows the session name and has a button to toggle
/// visibility.
class TreePanel::SessionRow_ {
  public:
    SessionRow_(const ContainerPane &pane);

    /// Sets the session text.
    void SetSessionText(const std::string &text);

  private:
    SwitcherPanePtr vis_switcher_pane_;  ///< Show/hide buttons.
    TextPanePtr     session_text_pane_;  ///< Session string TextPane.
};

TreePanel::SessionRow_::SessionRow_(const ContainerPane &pane) {
    vis_switcher_pane_ = pane.FindTypedPane<SwitcherPane>("VisSwitcher");
    session_text_pane_ = pane.FindTypedPane<TextPane>("SessionString");
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
    /// Creates a clone of the given ContainerPane and uses it to store
    /// information for the given Model.
    ModelRow_(const ContainerPane &pane, const Model &model);

    /// Returns the ContainerPane representing the row.
    ContainerPanePtr GetRowPane() const { return row_pane_; }

  private:
    ContainerPanePtr row_pane_;           ///< Pane for the row.
    SwitcherPanePtr  vis_switcher_pane_;  ///< Show/hide buttons.
    SwitcherPanePtr  exp_switcher_pane_;  ///< Expand/collapse buttons.
    PanePtr          spacer_pane_;        ///< Spacer to indent button_pane_.
    ButtonPanePtr    button_pane_;        ///< Model Name button.
    TextPanePtr      text_pane_;          ///< Model name TextPane in button.
};

TreePanel::ModelRow_::ModelRow_(const ContainerPane &pane, const Model &model) {
    row_pane_ = pane.CloneTyped<ContainerPane>(true);

    vis_switcher_pane_ = row_pane_->FindTypedPane<SwitcherPane>("VisSwitcher");
    exp_switcher_pane_ = row_pane_->FindTypedPane<SwitcherPane>("ExpSwitcher");
    spacer_pane_       = row_pane_->FindPane("Spacer");
    button_pane_       = row_pane_->FindTypedPane<ButtonPane>("ModelButton");
    text_pane_         = button_pane_->FindTypedPane<TextPane>("Text");

    text_pane_->SetText(model.GetName());

    // The Pane used to create this was disabled, so enable the clone.
    row_pane_->SetEnabled(true);
}

// ----------------------------------------------------------------------------
// TreePanel::Impl_ class definition.
// ----------------------------------------------------------------------------

class TreePanel::Impl_ {
  public:
    void Reset();
    void SetSessionString(const std::string &str);
    void SetRootModel(const RootModelPtr &root_model) {
        root_model_ = root_model;
    }

    /// Marks models as having changed in some way that requires rebuilding.
    void ModelsChanged() { models_changed_ = true; }

    /// Updates Model rows if any change was made.
    void UpdateModels() {
        if (models_changed_ && root_model_)
            UpdateModelRows_();
    }

    void InitInterface(ContainerPane &root_pane);


  private:
    /// Defines the current state for a ModelRow_.
    enum class ModelRowState_ {
        kNonGroup,   ///< Row represents a non-group, so cannot be expanded.
        kExpanded,   ///< Group is expanded to show children.
        kCollapsed,  ///< Group is collapsed to hide children.
    };

    typedef std::shared_ptr<TreePanel::ModelRow_>   ModelRowPtr_;
    typedef std::shared_ptr<TreePanel::SessionRow_> SessionRowPtr_;

    typedef std::unordered_map<ModelPtr, ModelRowState_> ModelRowStateMap_;

    RootModelPtr              root_model_;
    ScrollingPanePtr          scrolling_pane_;
    SessionRowPtr_            session_row_;
    ContainerPanePtr          model_row_pane_;
    std::vector<ModelRowPtr_> model_rows_;

    /// Maps a ModelPtr to the state of the ModelRow_ for that Model. This
    /// allows state to persist even when the rows are rebuilt from scratch.
    ModelRowStateMap_         model_row_state_map_;

    /// Indicates that Models have changed in some way that requires rebuild.
    bool                      models_changed_ = true;

    void UpdateModelRows_();

    /// Recursive function that adds a row for the given Model and each of its
    /// descendants to model_rows_.
    void AddModelRow_(const ModelPtr &model);

    /// Returns the current ModelRowState_ for a Model. Adds or updates a
    /// map entry as necessary.
    ModelRowState_ GetModelRowState_(const ModelPtr &model);
};

// ----------------------------------------------------------------------------
// TreePanel::Impl_ functions.
// ----------------------------------------------------------------------------

void TreePanel::Impl_::Reset() {
    model_rows_.clear();
    if (root_model_)
        UpdateModelRows_();
}

void TreePanel::Impl_::SetSessionString(const std::string &str) {
    ASSERT(session_row_);
    session_row_->SetSessionText(str);
}

void TreePanel::Impl_::InitInterface(ContainerPane &root_pane) {
    scrolling_pane_ = root_pane.FindTypedPane<ScrollingPane>("Scroller");

    // Set up the session row.
    auto session_row_pane = root_pane.FindTypedPane<ContainerPane>("SessionRow");
    session_row_.reset(new SessionRow_(*session_row_pane));

    // ModelRow_ instances will be created and added later. Save the Pane used
    // to create them
    model_row_pane_ = root_pane.FindTypedPane<ContainerPane>("ModelRow");
}

void TreePanel::Impl_::UpdateModelRows_() {
    ASSERT(root_model_);

    // There is no row for the root model. Add each of its children.
    for (size_t i = 0; i < root_model_->GetChildModelCount(); ++i)
        AddModelRow_(root_model_->GetChildModel(i));

    // Create Panes for each row and replace the contents of the ScrollingPane.
    std::vector<PanePtr> row_panes;
    for (const auto &row: model_rows_)
        row_panes.push_back(row->GetRowPane());
    ASSERT(scrolling_pane_->GetContentsPane());
    scrolling_pane_->GetContentsPane()->ReplacePanes(row_panes);

    models_changed_ = false;
}

void TreePanel::Impl_::AddModelRow_(const ModelPtr &model) {
    std::cerr << "XXXX Adding row for " << model->GetDesc() << "\n";
    ModelRowPtr_ row(new ModelRow_(*model_row_pane_, *model));
    model_rows_.push_back(row);

    // Recurse on children if necessary.
    const ModelRowState_ state = GetModelRowState_(model);
    if (state == ModelRowState_::kExpanded) {
        ASSERT(dynamic_cast<const ParentModel *>(model.get()));
        const ParentModel &parent = static_cast<const ParentModel &>(*model);
        for (size_t i = 0; i < parent.GetChildModelCount(); ++i)
            AddModelRow_(parent.GetChildModel(i));
    }
}

TreePanel::Impl_::ModelRowState_ TreePanel::Impl_::GetModelRowState_(
    const ModelPtr &model) {
    auto it = model_row_state_map_.find(model);
    if (it != model_row_state_map_.end())
        return it->second;

    // Groups are expanded by default.
    ModelRowState_ state = dynamic_cast<const ParentModel *>(model.get()) ?
        ModelRowState_::kExpanded : ModelRowState_::kNonGroup;
    model_row_state_map_[model] = state;
    return state;
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

void TreePanel::SetRootModel(const RootModelPtr &root_model) {
    impl_->SetRootModel(root_model);
}

void TreePanel::ModelsChanged() {
    impl_->ModelsChanged();
}

void TreePanel::UpdateForRenderPass(const std::string &pass_name) {
    impl_->UpdateModels();
}

void TreePanel::InitInterface() {
    impl_->InitInterface(*GetPane());
}
