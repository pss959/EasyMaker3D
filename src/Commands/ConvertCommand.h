#pragma once

#include <memory>

#include "Commands/MultiModelCommand.h"

/// ConvertCommand is an abstract base class for command classes that create a
/// ConvertedModel of some sort from one or more operand Models. It exists
/// solely for type information.
///
/// \ingroup Commands
class ConvertCommand : public MultiModelCommand {};

typedef std::shared_ptr<ConvertCommand> ConvertCommandPtr;
