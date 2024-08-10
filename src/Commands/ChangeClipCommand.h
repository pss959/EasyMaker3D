#pragma once

#include "Commands/ChangePlaneCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeClipCommand);

/// ChangeClipCommand is used to add a clipping Plane in one or more
/// ClippedModel instances. The side the Plane normal points to is clipped
/// away.
///
/// \ingroup Commands
class ChangeClipCommand : public ChangePlaneCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    ChangeClipCommand() {}

    friend class Parser::Registry;
};
