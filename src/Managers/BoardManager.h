#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Panels/PanelHelper.h"

DECL_SHARED_PTR(Board);
DECL_SHARED_PTR(BoardManager);
DECL_SHARED_PTR(PanelManager);

/// The BoardManager manages all current Board instances. It is derived from
/// PanelHelper so that Panel classes can operate on Board instances opaquely.
///
/// \ingroup Managers
class BoardManager : public PanelHelper {
  public:
    /// The BoardManager is passed a PanelManager used to access Panels.
    explicit BoardManager(const PanelManagerPtr &panel_manager);

    /// Clears all Boards in the manager.
    void Reset();

    /// Shows or hides the given Board. The Board's Behavior determines what
    /// happens with other visible Boards.
    void ShowBoard(const BoardPtr &board, bool is_shown);

    /// Returns the most recently shown non-permanent Board, or null if none is
    /// shown.
    BoardPtr GetCurrentBoard() const;

    // ------------------------------------------------------------------------
    // PanelHelper interface.
    // ------------------------------------------------------------------------
    virtual PanelPtr GetPanel(const std::string &name) const override;
    virtual void ClosePanel(const std::string &result) override;
    virtual void PushPanel(const PanelPtr &panel,
                           const ResultFunc &result_func) override;

  private:
    /// PanelManager used to access Panels.
    PanelManagerPtr panel_manager_;

    /// Currently active Board instances. The Behavior of each Board indicates
    /// whether Boards shown before it are visible. Note that any Board with
    /// Behavior::kPermanent is not added to the vector.
    std::vector<BoardPtr> boards_;

    /// Updates all non-permanent Boards when the visibility of the given Board
    /// changes.
    void UpdateBoards_(const BoardPtr &board);

    /// Changes Board visibility.
    static void ChangeBoardVisibility_(Board &board, bool is_shown);
};
