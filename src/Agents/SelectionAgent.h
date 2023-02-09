#pragma once

#include "Base/Memory.h"

class  Selection;
struct SelPath;
DECL_SHARED_PTR(SelectionAgent);

/// SelectionAgent is an abstract interface class that manages the currently
/// selected of 3D Models. All changes made through function calls are
/// considered to be atomic, meaning that the current selection is always
/// valid.
///
/// \ingroup Agents
class SelectionAgent {
  public:
    /// Returns the current selection.
    virtual const Selection & GetSelection() const = 0;

    /// Changes the selection to the given one.
    virtual void ChangeSelection(const Selection &new_selection) = 0;

    /// Changes the selection status for the Model on the given SelPath. The
    /// is_multi_select flag indicates whether multi-selection is in effect.
    virtual void ChangeModelSelection(const SelPath &path,
                                      bool is_multi_select) = 0;

    /// Deselects all currently-selected Models.
    virtual void DeselectAll() = 0;
};
