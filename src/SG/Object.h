#pragma once

#include "Base/Memory.h"
#include "Parser/Object.h"
#include "SG/Change.h"
#include "Util/Assert.h"
#include "Util/Notifier.h"

namespace SG {

DECL_SHARED_PTR(Object);

/// Object is an abstract base class for all SG scene graph classes.
///
/// \ingroup SG
class Object : public Parser::Object {
  public:
    virtual ~Object();

    /// Returns a Notifier that is invoked when a change is made to the Object.
    Util::Notifier<Change, const Object &> & GetChanged() { return changed_; }

    /// Returns a flag indicating whether the Object is static once created.
    /// This is used during cloning to indicate that the Object does not need
    /// to be cloned and can be instanced. The default is false.
    bool IsStatic() const { return is_static_; }

    /// Redefines this from Parser::Object to return false by default. Only
    /// certain types of SG objects are scoped.
    virtual bool IsScoped() const override { return false; }

    /// Enables or disables notification to observers. The default is enabled.
    void SetNotifyEnabled(bool enabled) { is_notify_enabled_ = enabled; }

    /// Returns whether notification to observers is enabled. The default is
    /// true but can be changed with SetNotifyEnabled().
    bool IsNotifyEnabled() const { return is_notify_enabled_; }

    /// Redefines this to return true only if IsStatic() returns false, meaning
    /// that there may be a reason to clone this object.
    virtual bool ShouldDeepClone() const { return ! IsStatic(); }

  protected:
    virtual void AddFields() override;
    /// Redefines this to log construction if enabled.
    virtual void ConstructionDone() override;

    /// The constructor is protected to make this abstract.
    Object() {}

    /// \name Notification functions
    ///@{

    /// Establishes a Util::Notifier observer connection from the given Object
    /// to this one. The observed Object will notify this one via
    /// ProcessChange() whenever it is modified.
    void Observe(Object &observed);

    /// Removes a connection created with Observe().
    void Unobserve(Object &observed);

    /// Returns true if Observe() was called for the given Object.
    bool IsObserving(Object &observed) const;

    /// This is called when an observed Object is modified. The original Object
    /// that caused the change is passed along. The Object class defines this
    /// to just log the Change. Derived classes can override this to add
    /// additional behavior. This returns false if notification is disabled and
    /// the change was not processed.
    virtual bool ProcessChange(Change change, const Object &obj);

    ///@}

    /// Allows derived classes to change the name of the Object.
    void ChangeName(const std::string &new_name) {
        SetName(new_name);
    }

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool> is_static_{"is_static", false};
    ///@}

    bool is_notify_enabled_ = true;   ///< Pass notification to observers.

    /// Notifies when a change is made to the Object or any of its observed
    /// Objects.
    Util::Notifier<Change, const Object &> changed_;
};

}  // namespace SG
