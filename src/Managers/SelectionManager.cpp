//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Managers/SelectionManager.h"

#include <ranges>

#include "Place/ClickInfo.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

SelectionManager::SelectionManager() {
}

SelectionManager::~SelectionManager() {
    if (root_model_)
        root_model_->GetTopLevelChanged().RemoveObserver(this);
}

void SelectionManager::SetRootModel(const RootModelPtr &root_model) {
    ASSERT(root_model);
    root_model_ = root_model;

    // Attach a callback to the RootModel to track when Models are added,
    // removed, hidden, or shown at the top level.
    root_model_->GetTopLevelChanged().AddObserver(
        this, [this](){ SelectionChanged_(Operation::kUpdate); });
}

void SelectionManager::ChangeSelection(const Selection &new_selection) {
    DeselectAllModels_(selection_);
    selection_ = CleanSelection_(new_selection);
    for (const auto &path: selection_.GetPaths())
        SelectModel_(path, path == selection_.GetPrimary());
    SelectionChanged_(Operation::kSelection);
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
        SelectionChanged_(Operation::kReselection);
}

void SelectionManager::SelectInDirection(Direction dir) {
    ASSERT(CanSelectInDirection(dir));
    SelPath path;
    GetSelectionInDirection_(dir, path);

    // Select only the new Model.
    ChangeSelection(Selection(path));
}

void SelectionManager::AttachClickToModel(Model &model) {
    auto func = [this, &model](const ClickInfo &info){
        ChangeModelSelection(SelPath(info.hit.path.GetSubPath(model)),
                             info.is_modified_mode);
    };
    model.GetClicked().AddObserver(this, func);
}

Selection SelectionManager::CleanSelection_(const Selection &sel) {
    // Going in reverse order (so that later ones win), add each one that is
    // not a duplicate, ancestor, or descendant to a new list.
    std::vector<SelPath> clean_paths;
    clean_paths.reserve(sel.GetCount());
    const auto &paths = sel.GetPaths();
    for (const auto &path: paths | std::views::reverse) {
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
    for (const auto &path: clean_paths | std::views::reverse)
        clean_sel.Add(path);
    return clean_sel;
}

void SelectionManager::DeselectAllModels_(const Selection &sel) {
    // Notify first for deselection.
    SelectionChanged_(Operation::kDeselection);

    // Set all top-level Models to Unselected if they are not hidden.
    for (size_t i = 0; i < root_model_->GetChildModelCount(); ++i) {
        Model &model = *root_model_->GetChildModel(i);
        if (model.GetStatus() != Model::Status::kHiddenByUser)
            model.SetStatus(Model::Status::kUnselected);
    }
}

void SelectionManager::SelectModel_(const SelPath &path, bool is_primary) {
    Model &model = *path.GetModel();

    // Set all ancestors to kDescendantShown status. If any was kUnselected,
    // set all of its children that are kAncestorShown to kUnselected.
    const auto &path_models = path.GetAllModels(true);  // Skip the root.
    for (auto &ancestor: path_models) {
        // Skip the end of the path (the selected Model).
        if (ancestor.get() != &model) {
            if (ancestor->GetStatus() == Model::Status::kUnselected) {
                ParentModelPtr ap =
                    std::dynamic_pointer_cast<ParentModel>(ancestor);
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
        ASSERT(path_models.size() >= 2U);
        ParentModelPtr parent = std::dynamic_pointer_cast<ParentModel>(
            path_models[path_models.size() - 2]);
        ASSERT(parent);
        for (size_t i = 0; i < parent->GetChildModelCount(); ++i) {
            Model &sibling = *parent->GetChildModel(i);
            if (&sibling != &model && ! sibling.IsSelected())
                sibling.SetStatus(Model::Status::kUnselected);
        }
    }

    // Set the status in the selected Model. Do this last in case the above
    // code set it to something else.
    model.SetStatus(is_primary ? Model::Status::kPrimary :
                    Model::Status::kSecondary);

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
        if (! primary.GetModel()->IsTopLevel()) {
            path = primary;
            path.pop_back();
        }
        break;

      case Direction::kFirstChild:
        if (ParentModelPtr parent =
            std::dynamic_pointer_cast<ParentModel>(primary.GetModel())) {
            if (parent->GetChildModelCount() > 0) {
                path = primary;
                path.push_back(parent->GetChildModel(0));
            }
        }
        break;

      case Direction::kPreviousSibling:
      case Direction::kNextSibling:
        if (! primary.GetModel()->IsTopLevel()) {
            ParentModelPtr parent = primary.GetParentModel();
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

void SelectionManager::SelectionChanged_(Operation op) {
#if ENABLE_DEBUG_FEATURES
    auto sel_string = [&](){
        Str s;
        for (const auto &sel_path: selection_.GetPaths()) {
            const auto model = sel_path.GetModel();
            if (! s.empty())
                s += ", ";
            s += "'" + model->GetName() + "' (L" +
                Util::ToString(model->GetLevel()) + ")";
        }
        return " { " + s + " }";
    };
    KLOG('S', "Selection change: " << Util::EnumName(op) << sel_string());
#endif
    selection_changed_.Notify(selection_, op);
}
