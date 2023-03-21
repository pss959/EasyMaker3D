#pragma once

#include <vector>

#include "Agents/BoardAgent.h"
#include "Base/Memory.h"

DECL_SHARED_PTR(Board);
DECL_SHARED_PTR(BoardManager);
DECL_SHARED_PTR(Frustum);
DECL_SHARED_PTR(PanelManager);

/// The BoardManager manages all current Board instances. It is derived from
/// BoardAgent so that Panel classes can operate on Board instances opaquely.
///
/// \ingroup Managers
class BoardManager : public BoardAgent {
  public:
    /// The BoardManager is passed a PanelManager used to access Panels.
    explicit BoardManager(const PanelManagerPtr &panel_manager);

    /// Clears all Boards in the manager.
    void Reset();

    /// Sets the Frustum to use for positioning floating Board instances.
    void SetFrustum(const FrustumPtr &frustum) { frustum_ = frustum; }

    /// Shows or hides the given Board. The Board's Behavior determines what
    /// happens with other visible Boards.
    void ShowBoard(const BoardPtr &board, bool is_shown);

    /// Returns the most recently shown non-permanent Board, or null if none is
    /// shown.
    BoardPtr GetCurrentBoard() const;

    // BoardAgent interface.
    virtual PanelPtr GetPanel(const std::string &name) const override;
    virtual void ClosePanel(const std::string &result) override;
    virtual void PushPanel(const PanelPtr &panel,
                           const ResultFunc &result_func) override;

    /// Uses GetPanel(), but requires that the Panel is of the given derived
    /// type. Asserts if not found.
    template <typename T>
    std::shared_ptr<T> GetTypedPanel(const std::string &name) const {
        auto panel = Util::CastToDerived<T>(GetPanel(name));
        ASSERT(panel);
        return panel;
    }

  private:
    /// PanelManager used to access Panels.
    PanelManagerPtr       panel_manager_;

    /// Currently active Board instances. The Behavior of each Board indicates
    /// whether Boards shown before it are visible. Note that any Board with
    /// Behavior::kPermanent is not added to the vector.
    std::vector<BoardPtr> boards_;

    /// Frustum used for floating Board positioning.
    FrustumPtr            frustum_;

    /// Updates all non-permanent Boards when the visibility of the given Board
    /// changes.
    void UpdateBoards_(const BoardPtr &board);

    /// Changes Board visibility.
    static void ChangeBoardVisibility_(Board &board, bool is_shown);
};
