#pragma once

#include "Commands/MultiModelCommand.h"
#include "Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CopyCommand);

/// CopyCommand is used to copy one or more Models to the clipboard. It is not
/// undoable.
///
/// \ingroup Commands
class CopyCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;
    virtual bool HasUndoEffect() const { return false; }
    virtual bool HasRedoEffect() const { return false; }

    /// CopyCommand could potentially affect other commands even if orphaned.
    virtual bool ShouldBeAddedAsOrphan() const { return true; }

  protected:
    CopyCommand() {}

  private:
    friend class Parser::Registry;
};
