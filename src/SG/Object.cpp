#include "SG/Object.h"

#include "Util/Assert.h"
#include "Util/KLog.h"
#include "Util/String.h"

namespace SG {

Object::~Object() {
    KLOG('c', "Destroying " << GetDesc());

    // Avoid issues with notification going through destroyed objects.
    SetNotifyEnabled(false);
}

void Object::AddFields() {
    AddField(is_static_);
}

void Object::ConstructionDone() {
    KLOG('c', "Constructed " << GetDesc() << (IsClone() ? " As CLONE" : ""));
}

void Object::Observe(Object &observed) {
    KLOG('o', GetDesc() << " observing " << observed.GetDesc());
    try {
        observed.changed_.AddObserver(
            this, [&](Change change,
                      const Object &obj){ ProcessChange(change, obj); });
    }
    catch (std::exception &) {
        // Throw assertion with a more precise error message.
        ASSERTM(false, "Failed to Observe " + observed.GetDesc() + " in " +
                GetDesc());
    }
}

void Object::Unobserve(Object &observed) {
    KLOG('o', GetDesc() << " unobserving  " << observed.GetDesc());
    observed.changed_.RemoveObserver(this);
}

bool Object::IsObserving(Object &observed) const {
    return observed.changed_.HasObserver(this);
}

obool Object::ProcessChange(Change change, const Object &obj) {
    // Do nothing if notification is disabled. This also prevents problems from
    // occurring when the Object is being destroyed.
    if (! IsNotifyEnabled()) {
        return false;
    }
    else {
        if (&obj == this) {
            KLOG('n', GetDesc() << " initiating change "
                 << Util::EnumName(change));
        }
        else {
            KLOG('N', "  " << GetDesc() << " got change "
                 << Util::EnumName(change) << " from " << obj.GetDesc());
        }

        // Pass notification to observers.
        changed_.Notify(change, obj);

        return true;
    }
}

}  // namespace SG
