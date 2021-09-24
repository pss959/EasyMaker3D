#pragma once

#include "Assert.h"
#include "Parser/Object.h"

namespace SG {

class Tracker;

/// Object is an abstract base class for all SG scene graph classes.
class Object : public Parser::Object {
  public:
    virtual ~Object();

  protected:
    /// Returns a flag indicating whether the instance is being destroyed. This
    /// can be used to prevent executing problematic actions resulting from
    /// notification during destruction.
    bool IsBeingDestroyed() { return is_being_destroyed_; }

    /// The constructor is protected to make this abstract.
    Object() {}

    /// Redefines this to log construction if enabled.
    virtual void ConstructionDone() override;

  private:
    bool is_being_destroyed_ = false;  ///< Set to true in destructor.
};

}  // namespace SG
