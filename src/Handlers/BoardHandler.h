#pragma once

#include <memory>

#include "Handlers/Handler.h"
#include "Items/Board.h"

/// The BoardHandler class passes all events to the Panel installed in a given
/// Board if there is a Panel and the Board is visible. This is used to allow
/// keyboard interaction with panels.
class BoardHandler : public Handler {
  public:
    /// Sets the Board to handle events for.
    void SetBoard(const BoardPtr &board);

    virtual bool HandleEvent(const Event &event) override;

    /// Redefines this to return false if there is no Board or it has no active
    /// Panel.
    virtual bool IsEnabled() const override;

  private:
    BoardPtr board_;
};

typedef std::shared_ptr<BoardHandler> BoardHandlerPtr;
