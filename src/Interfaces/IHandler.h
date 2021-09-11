#pragma once

#include "Interfaces/IInterfaceBase.h"

struct Event;

//! Abstract Interface base class for classes that handle Event instances
//! representing input events. (It's not a true interface because it has a
//! member variable and non-pure-virtual function.)
//!
//! \ingroup Interfaces
class IHandler : public IInterfaceBase {
  public:
    //! Processes the given Event. Returns true if nobody else should get a
    //! chance to handle the event.
    virtual bool HandleEvent(const Event &event) = 0;

    //! Enables or disables the handler. This class implements this to set a
    //! flag. Derived classes may add other functionality.
    virtual void SetEnabled(bool enabled) { is_enabled_ = enabled; }

    //! Returns whether the handler is enabled. This class implements it to
    //! return the flag set in the last call to Enable().
    virtual bool IsEnabled() { return is_enabled_; }

  private:
    //! Whether the handler is enabled.
    bool is_enabled_ = false;
};
