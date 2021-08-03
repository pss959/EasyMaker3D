#pragma once

//! Abstract base class for all abstract base Interface classes.
//! \ingroup Interfaces
class IInterfaceBase {
  public:
    //! Marks the destructor as private so that instances are destroyed
    //! properly.
    virtual ~IInterfaceBase() {}

    //! All classes can report their name for debugging purposes.
    virtual const char * GetClassName() const = 0;
};
