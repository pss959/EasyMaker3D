#pragma once

#include "Assert.h"
#include "Parser/Object.h"
#include "SG/Change.h"
#include "Util/Notifier.h"

namespace SG {

class Tracker;

/// Object is an abstract base class for all SG scene graph classes.
class Object : public Parser::Object {
  public:
    virtual ~Object();

    /// Allows the name of any SG object to be renamed. This is virtual to
    /// allow derived classes to add renaming behavior.
    virtual void ChangeName(const std::string &new_name) {
        SetName(new_name);
    }

    /// Returns a Notifier that is invoked when a change is made to the Object.
    Util::Notifier<Change> & GetChanged() { return changed_; }

  protected:
    /// Returns a flag indicating whether the instance is being destroyed. This
    /// can be used to prevent executing problematic actions resulting from
    /// notification during destruction.
    bool IsBeingDestroyed() { return is_being_destroyed_; }

    /// The constructor is protected to make this abstract.
    Object() {}

    /// Redefines this to log construction if enabled.
    virtual void ConstructionDone() override;

    /// \name Notification functions
    ///@{

    /// Establishes a Util::Notifier observer connection from the given Object
    /// to this one. The observed Object will notify this one via
    /// ProcessChange() whenever it is modified.
    void Observe(Object &observed);

    /// Removes a connection created with Observe().
    void Unobserve(Object &observed);

    /// This is called when an observed Object is modified. The Object class
    /// defines this to just log the Change. Derived classes can override this
    /// to add additional behavior.
    virtual void ProcessChange(Change change);

    ///@}

  private:
    bool is_being_destroyed_ = false;  ///< Set to true in destructor.

    /// Notifies when a change is made to the Object or any of its observed
    /// Objects.
    Util::Notifier<Change> changed_;
};

}  // namespace SG
