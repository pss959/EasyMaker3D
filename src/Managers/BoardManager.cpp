//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Managers/BoardManager.h"

#include <ranges>

#include "Managers/PanelManager.h"
#include "Math/Frustum.h"
#include "Panels/Board.h"
#include "Util/Assert.h"
#include "Util/KLog.h"
#include "Util/Tuning.h"

BoardManager::BoardManager(const PanelManagerPtr &panel_manager) :
    panel_manager_(panel_manager) {
    ASSERT(panel_manager);
}

void BoardManager::Reset() {
    boards_.clear();
}

void BoardManager::ShowBoard(const BoardPtr &board, bool is_shown) {
    ASSERT(board);

    // Show or hide the Board.
    ChangeBoardVisibility_(*board, is_shown);

    // Position the Board if it is floating. Note that none of this has any
    // effect when in VR.
    if (is_shown && board->IsFloating()) {
        // Use the Frustum to compute a good position and direction.
        ASSERT(frustum_);
        const Vector3f view_dir = frustum_->GetViewDirection();
        Point3f pos = frustum_->position + frustum_->position[2] * view_dir;
        board->SetOrientation(view_dir);

        // Also make sure it is above the stage (bottom above Y=0).
        const float min_y = pos[1] + board->GetScaledBounds().GetMinPoint()[1];
        if (min_y < 0)
            pos[1] += TK::kFloatingBoardYOffset - min_y;
        board->SetPosition(pos);
    }

    // Update other Boards based on the Board's behavior; permanent Boards have
    // no effect.
    if (board->GetBehavior() != Board::Behavior::kPermanent)
        UpdateBoards_(board);
}

BoardPtr BoardManager::GetCurrentBoard() const {
    return boards_.empty() ? BoardPtr() : boards_.back();
}

PanelPtr BoardManager::GetPanel(const Str &name) const {
    return panel_manager_->GetPanel(name);
}

void BoardManager::ClosePanel(const Str &result) {
    // Do not try to close a panel in a permanent Board. When this happens, the
    // boards_ vector is empty.
    if (! boards_.empty()) {
        // Use the most current active Board.
        auto &board = boards_.back();
        ASSERT(board->GetCurrentPanel());
        ASSERT(board->GetBehavior() != Board::Behavior::kPermanent);

        KLOG('g', "Closing " << board->GetCurrentPanel()->GetName()
             << " with result '" << result
             << "' in " << board->GetDesc() << " with behavior "
             << Util::EnumName(board->GetBehavior()));

        // If popping results in an empty Board, hide it.
        if (! board->PopPanel(result))
            ShowBoard(board, false);
    }
}

void BoardManager::PushPanel(const PanelPtr &panel,
                             const ResultFunc &result_func) {
    // Use the most current active Board.
    ASSERT(! boards_.empty());
    auto &board = boards_.back();
    board->PushPanel(panel, result_func);

    // Make sure the Board is visible.
    ShowBoard(board, true);
}

void BoardManager::UpdateBoards_(const BoardPtr &board) {
    const auto behavior = board->GetBehavior();

    // If showing the new Board, push it on the stack if it is not already
    // there. If its behavior is Behavior::kReplaces, hide all other boards
    // first.
    if (board->IsShown()) {
        if (boards_.empty() || boards_.back() != board) {
            if (behavior == Board::Behavior::kReplaces) {
                for (auto &bd: boards_)
                    ChangeBoardVisibility_(*bd, false);
            }
            boards_.push_back(board);
        }
    }

    // If hiding the Board, remove it from the back of the stack.  If its
    // behavior is Behavior::kReplaces, show the most recently shown board(s).
    else {
        ASSERT(! boards_.empty());
        ASSERT(boards_.back() == board);
        boards_.pop_back();

        if (behavior == Board::Behavior::kReplaces) {
            for (const auto &bd: boards_ | std::views::reverse) {
                ChangeBoardVisibility_(*bd, true);
                if (bd->GetBehavior() != Board::Behavior::kAugments)
                    break;
            }
        }
    }
}

void BoardManager::ChangeBoardVisibility_(Board &board, bool is_shown) {
    KLOG('g', (is_shown ? "Showing " : "Hiding ")
         << board.GetDesc() << " with behavior "
         << Util::EnumName(board.GetBehavior())
         << " and panel "
         << (board.GetCurrentPanel() ?
             board.GetCurrentPanel()->GetName() : "NONE"));
    board.Show(is_shown);
}
