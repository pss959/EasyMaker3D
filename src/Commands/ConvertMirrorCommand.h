#pragma once

#include "Base/Memory.h"
#include "Commands/ConvertCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ConvertMirrorCommand);

/// ConvertMirrorCommand is used to convert each selected Model that is not a
/// MirroredModel to a MirroredModel.
///
/// \ingroup Commands
class ConvertMirrorCommand : public ConvertCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    ConvertMirrorCommand() {}

  private:
    friend class Parser::Registry;
};
