#include "Parser/Object.h"

#include "Parser/Registry.h"
#include "Util/String.h"

namespace Parser {

std::string Object::GetDesc() const {
    std::string s = GetTypeName();
    if (! GetName().empty())
        s += " '" + GetName() + "'";
    s += " (" + Util::ToString(this) + ")";
    return s;
}

Field * Object::FindField(const std::string &name) const {
    for (auto &field: fields_)
        if (field->GetName() == name)
            return field;
    return nullptr;
}

ObjectPtr Object::Clone(bool is_deep) const {
    // Create the clone.
    ObjectPtr clone = Registry::CreateObjectOfType(GetTypeName(), GetName());
    ASSERT(clone);
    ASSERT(clone->GetTypeName() == GetTypeName());

    // Copy the fields.
    auto       &this_fields  = GetFields();
    const auto &clone_fields = clone->GetFields();
    ASSERT(this_fields.size() == clone_fields.size());
    for (size_t i = 0; i < this_fields.size(); ++i)
        clone_fields[i]->CopyFrom(*this_fields[i], is_deep);

    return clone;
}

}  // namespace Parser
