//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Panes/ContainerPane.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(SwitcherPane);

/// SwitcherPane is a derived ContainerPane that displays none or one of its
/// contained Panes at any time. The index field indicates which one is
/// displayed; a negative or too-large value turns off all display. The
/// SwitcherPane is sized to the maximum size of all of its contained Panes.
///
/// \ingroup Panes
class SwitcherPane : public ContainerPane {
  public:
    /// Sets the index of the contained Pane to display.
    void SetIndex(int index);

    /// Returns the index of the current Pane being displayed. If none is
    /// displayed, this returns -1.
    int GetIndex() const { return index_; }

    virtual Str ToString(bool is_brief) const override;

  protected:
    SwitcherPane() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

    virtual Vector2f ComputeBaseSize() const override;
    virtual void     LayOutSubPanes() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<int> index_;
    ///@}

    /// Updates when the index changes.
    void UpdateIndex_(int new_index, bool force_update);

    friend class Parser::Registry;
};
