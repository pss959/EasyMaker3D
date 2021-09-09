#pragma once

#include "Assert.h"
#include "Parser/Object.h"
#include "SG/Context.h"

namespace SG {

class Tracker;

//! Object is an abstract base class for all SG scene graph classes.
class Object : public Parser::Object {
  public:
    virtual ~Object();

    //! This is called after a scene is read to set up Ion types in all SG
    //! objects. Each derived class should implement this to recurse on all
    //! sub-objects and set up itself. The base class defines this to store the
    //! context in case anyone needs it later.
    virtual void SetUpIon(const ContextPtr &context) { context_ = context; }

  protected:
    //! Returns a flag indicating whether the instance is being destroyed. This
    //! can be used to prevent executing problematic actions resulting from
    //! notification during destruction.
    bool IsBeingDestroyed() { return is_being_destroyed_; }

    //! The constructor is protected to make this abstract.
    Object() {}

    //! Redefines this to log construction if enabled.
    virtual void ConstructionDone() override;

    //! Returns the Context for the object. This will be null until SetUpIon()
    //! is called for the first time.
    const ContextPtr & GetContext() const { return context_; }

  private:
    ContextPtr context_;
    bool is_being_destroyed_ = false; //!< Set to true in destructor.
};

}  // namespace SG
