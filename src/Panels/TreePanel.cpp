#include "Panels/TreePanel.h"

#include <functional>
#include <unordered_map>
#include <vector>

#include "Managers/ColorManager.h"
#include "Panes/ButtonPane.h"
#include "Panes/ContainerPane.h"
#include "Panes/ScrollingPane.h"
#include "Panes/SpacerPane.h"
#include "Panes/SwitcherPane.h"
#include "Panes/TextPane.h"
#include "SelPath.h"
#include "Util/Assert.h"
#include "Util/Enum.h"

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

    void SetSelectionManager(const SelectionManagerPtr &selection_manager) {
        selection_manager_ = selection_manager;
    }

  private:
    /// Defines the current visibility state for the session row or for a
    /// ModelRow_. Note that these values must be in the same order as the
    /// children in the vis_switcher_pane_.
    enum class VisState_ {
        kNotTopLevel,  ///< Models at deeper levels cannot be hidden by user.
        kVisible,      ///< Model is visible.
        kInvisible,    ///< Model was hidden by the user.
    };

    /// Defines the current expand/collapse state for a ModelRow_. Note that
    /// these values must be in the same order as the children in the
    /// exp_switcher_pane_.
    enum class ExpState_ {
        kNonGroup,   ///< Row represents a non-group, so cannot be expanded.
        kExpanded,   ///< Group is expanded to show children.
        kCollapsed,  ///< Group is collapsed to hide children.
    };

    /// A ModelRow_ instance represents one row of the tree view that displays
    /// the name of a Model and allows the TreePanel to interact with it.
    class ModelRow_ {
      public:
        /// Typedef for function passed to SetShowHideFunc().
        typedef std::function<void(ModelRow_ &, bool)> ShowHideFunc;

        /// Creates a clone of the given ContainerPane and uses it to store
        /// information for Model represented by the SelPath. The ExpState_ to
        /// use for the ModelRow_ is supplied.
        ModelRow_(const ContainerPane &pane, const SelPath &sel_path,
                  ExpState_ exp_state);

        /// Attaches the given callback to the show/hide buttons. The callback
        /// is passed the ModelRow_ and a flag that is true for show and false
        /// for hide.
        void SetShowHideFunc(const ShowHideFunc &func);

        /// Returns the ContainerPane representing the row.
        ContainerPanePtr GetRowPane() const { return row_pane_; }

        /// Returns the SelPath for the Model this row represents.
        const SelPath & GetSelPath() const { return sel_path_; }

        /// Updates the visibility button based on the Model's current state.
        void UpdateVisibility();

      private:
        ShowHideFunc     show_hide_func_;
        SelPath          sel_path_;           ///< Selection path to model.
        ContainerPanePtr row_pane_;           ///< Pane for the row.
        SwitcherPanePtr  vis_switcher_pane_;  ///< Show/hide buttons.
        SwitcherPanePtr  exp_switcher_pane_;  ///< Expand/collapse buttons.
        SpacerPanePtr    spacer_pane_;        ///< To indent button_pane_.
        ButtonPanePtr    button_pane_;        ///< Model Name button.
        TextPanePtr      text_pane_;          ///< TextPane in button.
        ExpState_        exp_state_;          ///< Expand/collapse state.
        VisState_        vis_state_;          ///< Visibility state.

        void Show_();
        void Hide_();

        std::string GetFontNameForModel_(const Model &model);
        Color       GetColorForModel_(const Model &model);
    };

    typedef std::shared_ptr<ModelRow_>              ModelRowPtr_;
    typedef std::unordered_map<ModelPtr, ExpState_> ExpStateMap_;

    SelectionManagerPtr       selection_manager_;
    RootModelPtr              root_model_;
    SwitcherPanePtr           session_vis_switcher_pane_;
    TextPanePtr               session_text_pane_;
    ScrollingPanePtr          scrolling_pane_;
    ContainerPanePtr          model_row_pane_;
    std::vector<ModelRowPtr_> model_rows_;

    /// Maps a ModelPtr to the ExpState_ of the ModelRow_ for that Model. This
    /// allows state to persist even when the rows are rebuilt from scratch.
    ExpStateMap_              exp_state_map_;

    /// Indicates that Models have changed in some way that requires rebuild.
    bool                      models_changed_ = true;

    void UpdateModelRows_();

    /// Recursive function that adds a row for the Model represented by the
    /// given SelPath and each of its descendants to model_rows_.
    void AddModelRow_(const SelPath &sel_path);

    /// Returns the current ExpState_ for a Model. Adds or updates a map entry
    /// as necessary.
    ExpState_ GetExpState_(const ModelPtr &model);

    /// Shows or hides the Model for the given ModelRow_.
    void ShowOrHideModel_(ModelRow_ &row, bool show);
};

// ----------------------------------------------------------------------------
// TreePanel::Impl_ functions.
// ----------------------------------------------------------------------------

void TreePanel::Impl_::Reset() {
    if (root_model_)
        UpdateModelRows_();
}

void TreePanel::Impl_::SetSessionString(const std::string &str) {
    ASSERT(session_text_pane_);
    if (session_text_pane_->GetText() != str)
        session_text_pane_->SetText(str);
}

void TreePanel::Impl_::InitInterface(ContainerPane &root_pane) {
    scrolling_pane_ = root_pane.FindTypedPane<ScrollingPane>("Scroller");

    // Set up the session row text.
    auto session_row_pane = root_pane.FindTypedPane<ContainerPane>("SessionRow");
    session_text_pane_ =
        session_row_pane->FindTypedPane<TextPane>("SessionString");

    // And its visibility switch with show/hide callbacks.
    auto vsp = session_row_pane->FindTypedPane<SwitcherPane>("VisSwitcher");
    auto show = vsp->FindTypedPane<ButtonPane>("ShowButton");
    auto hide = vsp->FindTypedPane<ButtonPane>("HideButton");
    show->GetButton().GetClicked().AddObserver(
        this, [&](const ClickInfo &){ root_model_->ShowAllModels(); });
    hide->GetButton().GetClicked().AddObserver(
        this, [&](const ClickInfo &){
            selection_manager_->DeselectAll();
            root_model_->HideAllModels();
        });
    session_vis_switcher_pane_ = vsp;

    // ModelRow_ instances will be created and added later. Save the Pane used
    // to create them
    model_row_pane_ = root_pane.FindTypedPane<ContainerPane>("ModelRow");
}

void TreePanel::Impl_::UpdateModelRows_() {
    ASSERT(root_model_);

    model_rows_.clear();

    // There is no row for the root model. Add each of its children.
    for (size_t i = 0; i < root_model_->GetChildModelCount(); ++i)
        AddModelRow_(SelPath(root_model_, root_model_->GetChildModel(i)));

    // Create Panes for each row and replace the contents of the ScrollingPane.
    std::vector<PanePtr> row_panes;
    for (const auto &row: model_rows_)
        row_panes.push_back(row->GetRowPane());
    ASSERT(scrolling_pane_->GetContentsPane());
    scrolling_pane_->GetContentsPane()->ReplacePanes(row_panes);

    // Update the visibility switch in the session row.
    const bool all_visible = root_model_->GetHiddenModelCount() == 0;
    VisState_ vis_state = all_visible ?
        VisState_::kVisible : VisState_::kInvisible;
    session_vis_switcher_pane_->SetIndex(Util::EnumInt(vis_state));

    models_changed_ = false;
}

void TreePanel::Impl_::AddModelRow_(const SelPath &sel_path) {
    const auto &model = sel_path.GetModel();
    const ExpState_ exp_state = GetExpState_(model);
    ModelRowPtr_ row(new ModelRow_(*model_row_pane_, sel_path, exp_state));
    row->SetShowHideFunc([&](ModelRow_ &row,
                             bool show){ ShowOrHideModel_(row, show); });
    model_rows_.push_back(row);

    // Recurse on children if necessary.
    if (exp_state == ExpState_::kExpanded) {
        ASSERT(dynamic_cast<const ParentModel *>(model.get()));
        const ParentModel &parent = static_cast<const ParentModel &>(*model);
        for (size_t i = 0; i < parent.GetChildModelCount(); ++i) {
            SelPath child_sel_path = sel_path;
            child_sel_path.push_back(parent.GetChildModel(i));
            AddModelRow_(child_sel_path);
        }
    }
}

TreePanel::Impl_::ExpState_ TreePanel::Impl_::GetExpState_(
    const ModelPtr &model) {
    auto it = exp_state_map_.find(model);
    if (it != exp_state_map_.end())
        return it->second;

    // Groups are expanded by default.
    ExpState_ exp_state = dynamic_cast<const ParentModel *>(model.get()) ?
        ExpState_::kExpanded : ExpState_::kNonGroup;
    exp_state_map_[model] = exp_state;
    return exp_state;
}

void TreePanel::Impl_::ShowOrHideModel_(ModelRow_ &row, bool show) {
    const SelPath &sel_path = row.GetSelPath();
    const ModelPtr &model = sel_path.GetModel();
    if (show) {
        root_model_->ShowModel(model);
    }
    else {
        // Deselect the Model if it is selected.
        ASSERT(selection_manager_);
        if (model->IsSelected())
            selection_manager_->ChangeModelSelection(sel_path, true);
        root_model_->HideModel(model);
    }
    row.UpdateVisibility();
}

// ----------------------------------------------------------------------------
// TreePanel::Impl_::ModelRow_ class functions.
// ----------------------------------------------------------------------------

TreePanel::Impl_::ModelRow_::ModelRow_(const ContainerPane &pane,
                                       const SelPath &sel_path,
                                       ExpState_ exp_state) {
    sel_path_ = sel_path;
    sel_path_.Validate();

    row_pane_ = pane.CloneTyped<ContainerPane>(true);

    vis_switcher_pane_ = row_pane_->FindTypedPane<SwitcherPane>("VisSwitcher");
    exp_switcher_pane_ = row_pane_->FindTypedPane<SwitcherPane>("ExpSwitcher");
    spacer_pane_       = row_pane_->FindTypedPane<SpacerPane>("Spacer");
    button_pane_       = row_pane_->FindTypedPane<ButtonPane>("ModelButton");
    text_pane_         = button_pane_->FindTypedPane<TextPane>("Text");

    const auto &model = sel_path_.GetModel();
    text_pane_->SetFontName(GetFontNameForModel_(*model));
    text_pane_->SetColor(GetColorForModel_(*model));
    text_pane_->SetText(model->GetName());

    // The Pane used to create this was disabled, so enable the clone.
    row_pane_->SetEnabled(true);

    // Indent the Model based on its level. Note that level 0 is used for the
    // RootModel, so start at 1.
    if (model->GetLevel() > 1)
        spacer_pane_->SetSpace(Vector2f(model->GetLevel() * 4, 0));

    // Set up the expand/collapse state and callbacks.
    exp_state_ = exp_state;
    exp_switcher_pane_->SetIndex(Util::EnumInt(exp_state));

    // Set up the visibility state and callbacks.
    UpdateVisibility();
}

void TreePanel::Impl_::ModelRow_::SetShowHideFunc(const ShowHideFunc &func) {
    ASSERT(func);
    show_hide_func_ = func;
    auto show = vis_switcher_pane_->FindTypedPane<ButtonPane>("ShowButton");
    auto hide = vis_switcher_pane_->FindTypedPane<ButtonPane>("HideButton");
    show->GetButton().GetClicked().AddObserver(
        this, [&](const ClickInfo &){ show_hide_func_(*this, true); });
    hide->GetButton().GetClicked().AddObserver(
        this, [&](const ClickInfo &){ show_hide_func_(*this, false); });
}

void TreePanel::Impl_::ModelRow_::UpdateVisibility() {
    const ModelPtr &model = sel_path_.GetModel();
    vis_state_ = ! model->IsTopLevel() ? VisState_::kNotTopLevel :
        model->GetStatus() == Model::Status::kHiddenByUser ?
        VisState_::kInvisible : VisState_::kVisible;
    vis_switcher_pane_->SetIndex(Util::EnumInt(vis_state_));
}

std::string TreePanel::Impl_::ModelRow_::GetFontNameForModel_(
    const Model &model) {
    std::string font_name = "Verdana";

    // Use bold for selected models.
    if (model.IsSelected())
        font_name += "_Bold";

    // Use italic if hidden for some reason.
    if (! model.IsShown())
        font_name += "_Italic";

    return font_name;
}

Color TreePanel::Impl_::ModelRow_::GetColorForModel_(const Model &model) {
    std::string color_name;
    switch (model.GetStatus()) {
      case Model::Status::kUnselected:
        color_name = "Default";
        break;
      case Model::Status::kPrimary:
        color_name = "Primary";
        break;
      case Model::Status::kSecondary:
        color_name = "Secondary";
        break;
      case Model::Status::kHiddenByUser:
        color_name = "HiddenByUser";
        break;
      case Model::Status::kAncestorShown:
      case Model::Status::kDescendantShown:
        color_name = "HiddenByModel";
        break;
      default:
        ASSERTM(false, model.GetDesc() + " has invalid status");
    }
    return ColorManager::GetSpecialColor("TreePanel" + color_name + "Color");
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

void TreePanel::SetContext(const ContextPtr &context) {
    Panel::SetContext(context);
    impl_->SetSelectionManager(context->selection_manager);
}

void TreePanel::UpdateForRenderPass(const std::string &pass_name) {
    impl_->UpdateModels();
}

void TreePanel::InitInterface() {
    impl_->InitInterface(*GetPane());
}
