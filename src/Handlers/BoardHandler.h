//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Handlers/Handler.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(Board);
DECL_SHARED_PTR(BoardHandler);

/// The BoardHandler class passes all events to the Panel installed in a given
/// Board if there is a Panel and the Board is visible. This is used to allow
/// keyboard interaction with panels.
///
/// \ingroup Handlers
class BoardHandler : public Handler {
  public:
    /// Adds a Board to handle events for. The first visible Board is asked to
    /// handle events.
    void AddBoard(const BoardPtr &board);

    /// Clears the current list of Board instances to handle events for.
    void ClearBoards() { boards_.clear(); }

    virtual HandleCode HandleEvent(const Event &event) override;

    /// Redefines this to return false if there is no Board or it has no active
    /// Panel.
    virtual bool IsEnabled() const override;

  private:
    std::vector<BoardPtr> boards_;

    /// Returns the first visible Board, if any.
    BoardPtr GetFirstBoard_() const;
};
