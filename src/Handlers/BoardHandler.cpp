//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Handlers/BoardHandler.h"

#include "Panels/Board.h"
#include "Panels/Panel.h"
#include "Util/Assert.h"

void BoardHandler::AddBoard(const BoardPtr &board) {
    ASSERT(board);
    boards_.push_back(board);
}

Handler::HandleCode BoardHandler::HandleEvent(const Event &event) {
    // Check all Boards in order. Let only the first visible Board handle each
    // event.
    if (auto board = GetFirstBoard_()) {
        ASSERT(board->GetCurrentPanel());
        if (board->GetCurrentPanel()->HandleEvent(event))
            return HandleCode::kHandledStop;
    }
    return HandleCode::kNotHandled;
}

bool BoardHandler::IsEnabled() const {
    return Handler::IsEnabled() && GetFirstBoard_();
}

BoardPtr BoardHandler::GetFirstBoard_() const {
    for (auto &board: boards_)
        if (board->IsShown())
            return board;
    return BoardPtr();
}
