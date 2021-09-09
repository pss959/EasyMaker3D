#include "SG/Object.h"

#include "Util/KLog.h"

namespace SG {

Object::~Object() {
    is_being_destroyed_ = true;
    KLOG('c', "Destroying " << GetTypeName()
         << " (" << GetName() << ") " << this);
}

void Object::ConstructionDone() {
    KLOG('c', "Constructed " << GetTypeName()
         << " (" << GetName() << ") " << this);
}

}  // namespace SG
