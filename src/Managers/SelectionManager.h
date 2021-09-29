#pragma once

#include <memory>

#include "Models/RootModel.h"
#include "Selection.h"
#include "Util/Notifier.h"

/// SelectionManager manages the selection of 3D Models. It stores a single
/// primary selection and any number (including zero) of secondary selections.
///
/// All changes made through function calls are considered to be atomic, meaning
/// that the current selection is always valid.
///
/// \ingroup Managers
class SelectionManager {
  public:
    // ------------------------------------------------------------------------
    // Enums.
    // ------------------------------------------------------------------------

    /// Selection directions used for SelectInDirection().
    enum class Direction {
        kParent,
        kFirstChild,
        kPreviousSibling,
        kNextSibling,
    };

    /// Operations passed to the GetSelectionChanged() notifier.
    enum Operation {
        kSelection,    ///< The selection was changed.
        kReselection,  ///< All selected Models were reselected for update.
        kDeselection,  ///< All selected Models were deselected.
        kUpdate,       ///< No change in selection, but something else changed.
    };

    // ------------------------------------------------------------------------
    // Notifier.
    // ------------------------------------------------------------------------

    /// Returns a Notifier that is invoked when the selection changes. It is
    /// passed the new Selection and the Operation that caused the change.
    Util::Notifier<const Selection &, Operation> & GetSelectionChanged() {
        return selection_changed_;
    }

    // ------------------------------------------------------------------------
    // Initialization.
    // ------------------------------------------------------------------------

    /// The constructor is passed the RootModel.
    SelectionManager(const RootModelPtr &root_model);

    ~SelectionManager();

    /// Resets the SelectionManager as if the application just started.
    void Reset() {
        DeselectAll();
    }

    // ------------------------------------------------------------------------
    // Selection query.
    // ------------------------------------------------------------------------

    /// Returns the current selection.
    const Selection & GetSelection() const { return selection_; }

    /// Returns true if the primary selection can be changed in the given
    /// direction.
    bool CanSelectInDirection(Direction dir) const {
        SelPath path;
        return GetSelectionInDirection_(dir, path);
    }

    // ------------------------------------------------------------------------
    // Selection and deselection.
    // ------------------------------------------------------------------------

    /// Changes the selection to the given one.
    void ChangeSelection(const Selection &new_selection);

    /// Changes the selection status for the Model on the given SelPath. The
    /// is_multi_select flag indicates whether multi-selection is in effect.
    void ChangeModelSelection(const SelPath &path, bool is_multi_select);

    /// Selects all visible top-level models.
    void SelectAll();

    /// Deselects all currently-selected Models.
    void DeselectAll();

    /// Reselects all currently-selected Models. This can be used to update
    /// attached Tools after a change was made.
    void ReselectAll();

    /// Changes the primary selection according to the given direction. Asserts
    /// if that is not possible.
    void SelectInDirection(Direction dir);

    // ------------------------------------------------------------------------
    // Miscellaneous.
    // ------------------------------------------------------------------------

    /// Attaches a callback that selects or deselects a Model when clicked.
    void AttachClickToModel(Model &model);

  private:
    RootModelPtr root_model_;  ///< RootModel used to access Models.
    Selection    selection_;   ///< The current selection.

    /// Notifies when the selection changes.
    Util::Notifier<const Selection &, Operation> selection_changed_;

    /// Returns a "clean" version of the given Selection, removing any
    /// duplicates and selections that are ancestors or descendants of another
    /// selected Model. Models later in the selection take precedence over
    /// earlier ones.
    static Selection CleanSelection_(const Selection &sel);

    /// Deselects all Models in the given Selection.
    void DeselectAllModels_(const Selection &sel);

    /// Selects the Model in the given SelPath.
    void SelectModel_(const SelPath &path, bool is_primary);

    /// Sets path to a SelPath to the Model that is in the given direction
    /// relative to the primary selection. If there is no selection or no such
    /// relative, this just returns false.
    bool GetSelectionInDirection_(Direction dir, SelPath &path) const;
};

typedef std::shared_ptr<SelectionManager> SelectionManagerPtr;
