#include "SG/Object.h"

#include "Util/KLog.h"
#include "Util/String.h"

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

std::string Object::GetDesc() const {
    std::string s = GetTypeName();
    ASSERT(! s.empty());
    if (! GetName().empty())
        s += " '" + GetName() + "'";
    s += " (" + Util::ToString(this) + ")";
    return s;
}

}  // namespace SG
