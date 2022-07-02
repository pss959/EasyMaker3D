#include "Handlers/BoardHandler.h"

#include "Items/Board.h"
#include "Panels/Panel.h"
#include "Util/Assert.h"

void BoardHandler::AddBoard(const BoardPtr &board) {
    ASSERT(board);
    boards_.push_back(board);
}

bool BoardHandler::HandleEvent(const Event &event) {
    // Check all Boards in order. Let only the first visible Board handle each
    // event.
    if (auto board = GetFirstBoard_()) {
        ASSERT(board->GetCurrentPanel());
        return board->GetCurrentPanel()->HandleEvent(event);
    }
    return false;
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
