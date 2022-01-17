#include "Parser/Object.h"

#include "Parser/Registry.h"
#include "Util/KLog.h"
#include "Util/String.h"

namespace Parser {

std::string Object::GetDesc() const {
    std::string s = GetTypeName();
    if (! GetName().empty())
        s += " '" + GetName() + "'";
    if (IsClone() || IsTemplate())
        s += std::string(" [") + (IsTemplate() ? "T" : "") +
            (IsClone() ? "C" : "") + "]";
    s += " (" + Util::ToString(this) + ")";
    return s;
}

Field * Object::FindField(const std::string &name) const {
    for (auto &field: fields_)
        if (field->GetName() == name)
            return field;
    return nullptr;
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

void Object::SetUp_(const std::string &type_name, const std::string &name,
                    bool is_complete) {
    ASSERT(instance_type_ == InstanceType_::kUnknown);
    SetTypeName(type_name);
    SetName(name);
    ConstructionDone();
    AddFields();
    if (is_complete)
        CompleteInstance_(InstanceType_::kRegular);
}

ObjectPtr Object::Clone_(const std::string &name, bool is_deep,
                         bool is_complete) const {
    // Create the clone.
    ObjectPtr clone = Registry::CreateObjectOfType_(
        GetTypeName(), name.empty() ? GetName() : name, false);
    ASSERT(clone);
    clone->CopyContentsFrom(*this, is_deep);
    if (is_complete)
        clone->CompleteInstance_(InstanceType_::kClone);
    return clone;
}

void Object::CompleteInstance_(InstanceType_ instance_type) {
    ASSERT(instance_type_ == InstanceType_::kUnknown);
    ASSERT(instance_type  != InstanceType_::kUnknown);
    instance_type_ = instance_type;

    // Let the object know that parsing is done. This is needed for some
    // templates as well as regular instances.
    KLOG('c', "Calling CreationDone() for " << GetDesc());
    CreationDone();
}

}  // namespace Parser
