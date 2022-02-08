#pragma once

#include <memory>

#include "Commands/MultiModelCommand.h"

/// CombineCommand is an abstract base class for command classes that create a
/// CombinedModel of some sort from one or more operand Models. It exists
/// solely for type information.
/// \ingroup Commands
class CombineCommand : public MultiModelCommand {};
