#include "SG/Object.h"

#include "Assert.h"
#include "Util/KLog.h"
#include "Util/String.h"

namespace SG {

Object::~Object() {
    is_being_destroyed_ = true;
    KLOG('c', "Destroying " << GetTypeName()
         << " (" << GetName() << ") " << this);
}

void Object::AddFields() {
    AddField(is_static_);
}

void Object::ConstructionDone() {
    KLOG('c', "Constructed " << GetTypeName()
         << " (" << GetName() << ") " << this);
}

void Object::Observe(Object &observed) {
    KLOG('n', GetDesc() << " observing " << observed.GetDesc());
    try {
        observed.changed_.AddObserver(
            this, std::bind(&Object::ProcessChange, this,
                            std::placeholders::_1));
    }
    catch (std::exception &) {
        // Throw assertion with a more precise error message.
        ASSERTM(false, "Failed to Observe " + observed.GetDesc() + " in " +
                GetDesc());
    }
}

void Object::Unobserve(Object &observed) {
    KLOG('n', GetDesc() << " unobserving  " << observed.GetDesc());
    observed.changed_.RemoveObserver(this);
}

bool Object::IsObserving(Object &observed) const {
    return observed.changed_.HasObserver(this);
}

void Object::ProcessChange(Change change) {
    // Prevent crashes during destruction.
    if (IsBeingDestroyed())
        return;

    KLOG('n', GetDesc() << " got change " << Util::EnumName(change));

    // Pass notification to observers.
    if (IsNotifyEnabled())
        changed_.Notify(change);
}

}  // namespace SG
