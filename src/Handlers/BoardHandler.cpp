#include "Handlers/BoardHandler.h"

#include "Util/Assert.h"

void BoardHandler::SetBoard(const BoardPtr &board) {
    ASSERT(board);
    board_ = board;
}

bool BoardHandler::HandleEvent(const Event &event) {
    // Panel may have closed before this event could be processed.
    return IsEnabled() ? board_->GetPanel()->HandleEvent(event) : false;
}

bool BoardHandler::IsEnabled() const {
    return board_ && board_->GetPanel() && Handler::IsEnabled();
}
