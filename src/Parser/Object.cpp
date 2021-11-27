#include "Parser/Object.h"

#include "Parser/Registry.h"
#include "Util/String.h"

namespace Parser {

std::string Object::GetDesc() const {
    std::string s;
    if (GetObjectType() == ObjType::kTemplate)
        s = "TEMPLATE ";
    else if (GetObjectType() == ObjType::kInstance)
        s = "INSTANCE ";
    s += GetTypeName();
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

ObjectPtr Object::Clone(bool is_deep, const std::string &name) const {
    // Create the clone.
    ObjectPtr clone =
        Registry::CreateObjectOfType(GetTypeName(),
                                     name.empty() ? GetName() : name);
    ASSERT(clone);
    clone->SetObjectType(ObjType::kClone);
    clone->CopyContentsFrom(*this, is_deep);
    clone->ConstructionDone();
    return clone;
}

void Object::CopyContentsFrom(const Object &from, bool is_deep) {
    ASSERT(GetTypeName() == from.GetTypeName());
    // Copy the fields.
    auto       &from_fields  = from.GetFields();
    const auto &clone_fields = GetFields();
    ASSERT(from_fields.size() == clone_fields.size());
    for (size_t i = 0; i < from_fields.size(); ++i)
        clone_fields[i]->CopyFrom(*from_fields[i], is_deep);
}

}  // namespace Parser
