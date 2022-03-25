#pragma once

#include "Commands/MultiModelCommand.h"
#include "Memory.h"

DECL_SHARED_PTR(ConvertCommand);

/// ConvertCommand is an abstract base class for command classes that create a
/// ConvertedModel of some sort from one or more operand Models. It exists
/// solely for type information.
///
/// \ingroup Commands
class ConvertCommand : public MultiModelCommand {};
