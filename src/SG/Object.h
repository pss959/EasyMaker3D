#pragma once

#include "Assert.h"
#include "Parser/Object.h"
#include "SG/Context.h"

namespace SG {

class Tracker;

//! Object is an abstract base class for all SG scene graph classes.
class Object : public Parser::Object {
  public:
    virtual ~Object() {}

    //! This is called after a scene is read to set up Ion types in all SG
    //! objects. Each derived class should implement this to recurse on all
    //! sub-objects and set up itself. The base class defines this to store the
    //! context in case anyone needs it later.
    virtual void SetUpIon(const ContextPtr &context) { context_ = context; }

  protected:
    //! The constructor is protected to make this abstract.
    Object() {}

    //! Returns the Context for the object. This will be null until SetUpIon()
    //! is called for the first time.
    const ContextPtr & GetContext() const { return context_; }

  private:
    ContextPtr context_;
};

}  // namespace SG
