#include "Handlers/BoardHandler.h"

#include "Util/Assert.h"

void BoardHandler::AddBoard(const BoardPtr &board) {
    ASSERT(board);
    boards_.push_back(board);
}

bool BoardHandler::HandleEvent(const Event &event) {
    // Check all boards in order.
    for (auto &board: boards_) {
        if (board->IsShown())
            return board->GetPanel()->HandleEvent(event);
    }
    return false;
}

bool BoardHandler::IsEnabled() const {
    if (Handler::IsEnabled()) {
        for (auto &board: boards_) {
            if (board->IsShown())
                return true;
        }
    }
    return false;
}
