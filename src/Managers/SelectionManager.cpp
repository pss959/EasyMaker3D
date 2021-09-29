#include "Managers/SelectionManager.h"

#include "Assert.h"
#include "ClickInfo.h"

SelectionManager::SelectionManager(const RootModelPtr &root_model) :
    root_model_(root_model) {
    ASSERT(root_model_);

    // Attach a callback to the RootModel to track when Models are added,
    // removed, hidden, or shown at the top level.
    root_model_->GetTopLevelChanged().AddObserver(
        this, [this](){ selection_changed_.Notify(selection_,
                                                  Operation::kUpdate); });
}

SelectionManager::~SelectionManager() {
    root_model_->GetTopLevelChanged().RemoveObserver(this);
}

void SelectionManager::ChangeSelection(const Selection &new_selection) {
    DeselectAllModels_(selection_);
    selection_ = CleanSelection_(new_selection);
    for (const auto &path: selection_.GetPaths())
        SelectModel_(path, path == selection_.GetPrimary());
    selection_changed_.Notify(selection_, Operation::kSelection);
}

void SelectionManager::ChangeModelSelection(const SelPath &path,
                                            bool is_multi_select) {
    const bool is_selected = path.GetModel()->IsSelected();

    // Multi-object selection: toggle the Model's selection state.
    if (is_multi_select) {
        Selection sel;
        if (is_selected) {
            // To deselect, create a new Selection without it.
            for (const auto &p: selection_.GetPaths())
                if (p != path)
                    sel.Add(p);
        }
        else {
            // To select, add to the current selection.
            sel = selection_;
            sel.Add(path);
        }
        ChangeSelection(sel);
    }
    // Regular selection: select only the Model if it is not selected.
    else if (! is_selected) {
        ChangeSelection(Selection(path));
    }
}

void SelectionManager::SelectAll() {
    // If the primary selection is not a top-level Model, select its top-level
    // ancestor.
    while (CanSelectInDirection(Direction::kParent))
        SelectInDirection(Direction::kParent);

    // Select each top-level Model that is not hidden and is not already
    // selected. This will maintain selection order better than deselecting
    // everything and selecting all top-level Models from scratch.
    Selection sel = selection_;
    for (size_t i = 0; i < root_model_->GetChildModelCount(); ++i) {
        const ModelPtr &model = root_model_->GetChildModel(i);
        if (model->GetStatus() != Model::Status::kHiddenByUser &&
            ! model->IsSelected()) {
            SelPath path;
            path.push_back(root_model_);
            path.push_back(model);
            sel.Add(path);
        }
    }
    ChangeSelection(sel);
}

void SelectionManager::DeselectAll() {
    DeselectAllModels_(selection_);
    selection_.Clear();
}

void SelectionManager::ReselectAll() {
    // No changes to make - just notify.
    if (selection_.HasAny())
        selection_changed_.Notify(selection_, Operation::kReselection);
}

void SelectionManager::SelectInDirection(Direction dir) {
    ASSERT(CanSelectInDirection(dir));
    SelPath path;
    GetSelectionInDirection_(dir, path);

    // Select only the new Model.
    ChangeSelection(Selection(path));
}

void SelectionManager::AttachClickToModel(Model &model) {
#if XXXX
    // XXXX Need path to Model in ClickInfo!!!
    auto func = [this](const ClickInfo &){
        ChangeModelSelection(info.isAlternateMode));
    };
    model.GetClicked.AddObserver(func);
#endif
}

Selection SelectionManager::CleanSelection_(const Selection &sel) {
    // Going in reverse order (so that later ones win), add each one that is
    // not a duplicate, ancestor, or descendant to a new list.
    std::vector<SelPath> clean_paths;
    clean_paths.reserve(sel.GetCount());
    const auto &paths = sel.GetPaths();
    for (auto it = paths.rbegin(); it != paths.rend(); ++it) {
        const auto &path = *it;
        // Add the path if its Model is not a duplicate, ancestor, or
        // descendant of a Model already in the clean list.
        bool add_it = true;
        for (const auto &clean_path: clean_paths) {
            if (path == clean_path || clean_path.IsAncestorOf(path) ||
                path.IsAncestorOf(clean_path)) {
                add_it = false;
                break;
            }
        }
        if (add_it)
            clean_paths.push_back(path);
    }

    // Add paths in reverse order (to restore original order) to a new
    // Selection.
    Selection clean_sel;
    for (auto it = clean_paths.rbegin(); it != clean_paths.rend(); ++it)
        clean_sel.Add(*it);
    return clean_sel;
}

void SelectionManager::DeselectAllModels_(const Selection &sel) {
    // Notify first for deselection.
    selection_changed_.Notify(selection_, Operation::kDeselection);

    // Set all top-level Models to Unselected if they are not hidden.
    for (size_t i = 0; i < root_model_->GetChildModelCount(); ++i) {
        Model &model = *root_model_->GetChildModel(i);
        if (model.GetStatus() != Model::Status::kHiddenByUser)
            model.SetStatus(Model::Status::kUnselected);
    }
}

void SelectionManager::SelectModel_(const SelPath &path, bool is_primary) {
    Model &model = *path.GetModel();

    // Set the status in the selected Model.
    model.SetStatus(is_primary ? Model::Status::kPrimary :
                    Model::Status::kSecondary);

    // Set all ancestors to kDescendantShown status. If any was kUnselected,
    // set all of its children that are kAncestorShown to kUnselected.
    const auto &path_models = path.GetAllModels();
    for (auto &ancestor: path_models) {
        // Skip the last model.
        if (ancestor.get() != &model) {
            if (ancestor->GetStatus() == Model::Status::kUnselected) {
                ParentModelPtr ap = Util::CastToDerived<ParentModel>(ancestor);
                ASSERT(ap);
                for (size_t i = 0; i < ap->GetChildModelCount(); ++i) {
                    Model &child = *ap->GetChildModel(i);
                    if (child.GetStatus() == Model::Status::kAncestorShown)
                        child.SetStatus(Model::Status::kUnselected);
                }
            }
            ancestor->SetStatus(Model::Status::kDescendantShown);
        }
    }

    // Update all siblings to the kUnselected state to make them visible.
    if (! model.IsTopLevel()) {
        // The Model is at the end of the path, so its parent is just before
        // it.
        ASSERT(path_models.size() > 2U);
        ParentModelPtr parent = Util::CastToDerived<ParentModel>(
            path_models[path_models.size() - 2]);
        ASSERT(parent);
        for (size_t i = 0; i < parent->GetChildModelCount(); ++i) {
            Model &sibling = *parent->GetChildModel(i);
            if (&sibling != &model && ! sibling.IsSelected())
                sibling.SetStatus(Model::Status::kUnselected);
        }
    }
}

bool SelectionManager::GetSelectionInDirection_(Direction dir,
                                                SelPath &path) const {
    if (! selection_.HasAny())
        return false;
    const auto &primary = selection_.GetPrimary();

    // Start with an empty path. If anything is selected, it will be filled in.
    path.clear();

    switch (dir) {
      case Direction::kParent:
        if (primary.size() > 1U) {
            path = primary;
            path.pop_back();
        }
        break;

      case Direction::kFirstChild:
        if (ParentModelPtr parent =
            Util::CastToDerived<ParentModel>(primary.GetModel())) {
            if (parent->GetChildModelCount() > 0) {
                path = primary;
                path.push_back(parent->GetChildModel(0));
            }
        }
        break;

      case Direction::kPreviousSibling:
      case Direction::kNextSibling:
        if (primary.size() >= 2U) {
            ParentModelPtr parent =
                Util::CastToDerived<ParentModel>(primary[primary.size() - 2]);
            if (parent->GetChildModelCount() > 1U) {
                int index = parent->GetChildModelIndex(primary.GetModel());
                ASSERT(index >= 0);
                if (dir == Direction::kPreviousSibling)
                    index = index > 0 ? index - 1 :
                        parent->GetChildModelCount() - 1;
                else
                    index = (index + 1) % parent->GetChildModelCount();
                path = primary;
                path.pop_back();
                path.push_back(parent->GetChildModel(index));
            }
        }
        break;
    }

    return ! path.empty();
}
