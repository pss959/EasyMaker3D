#pragma once

#include "Commands/ChangePlaneCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeMirrorCommand);

/// ChangeMirrorCommand is used to add a mirroring Plane in one or more
/// MirroredModel instances.
///
/// \ingroup Commands
class ChangeMirrorCommand : public ChangePlaneCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    ChangeMirrorCommand() {}

    friend class Parser::Registry;
};
