#pragma once

#include "Interfaces/IInterfaceBase.h"

class Event;

//! Abstract Interface base class for classes that handle Event instances
//! representing input events.
//! \ingroup Interfaces
class IHandler : public IInterfaceBase {
  public:
    //! Processes the given Event. Returns true if nobody else should get a
    //! chance to handle the event.
    virtual bool HandleEvent(const Event &event) = 0;
};
