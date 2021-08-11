#pragma once

#include <string>

#include <ion/gfx/node.h>

#include "ExceptionBase.h"
#include "Interfaces/IInterfaceBase.h"

//! Abstract Interface class that can load items from files. Failure of any
//! loading function results in a Loader::Exception being thrown.
//! \ingroup Interfaces
class ILoader : public IInterfaceBase {
  public:
    //! Exception thrown when any loading function fails. Derived classes can
    //! create derived versions.
    typedef ExceptionBase Exception;

    //! Loads the contents of the file with the given path into a string and
    //! returns it.
    virtual std::string LoadFile(const std::string &path) = 0;

    //! Loads an Ion Node subgraph from the file with the given path, returning
    //! a NodePtr to the result.
    virtual ion::gfx::NodePtr LoadNode(const std::string &path) = 0;
};
