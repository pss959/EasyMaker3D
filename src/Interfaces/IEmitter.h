#pragma once

#include <vector>

#include <ion/math/vector.h>

#include "Interfaces/IInterfaceBase.h"

class Event;

//! Abstract Interface base class for classes that emit Event instances
//! representing input events.
//! \ingroup Interfaces
class IEmitter : public IInterfaceBase {
  public:
    //! Adds emitted Event instances to the given vector.
    virtual void EmitEvents(std::vector<Event> &events) = 0;
};
