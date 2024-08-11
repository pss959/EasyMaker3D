//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Parser/Registry.h"

#include "Parser/Exception.h"
#include "Util/General.h"

namespace Parser {

Registry::TypeNameMap_ Registry::type_name_map_;
Registry::CreationMap_ Registry::creation_map_;

StrVec Registry::GetAllTypeNames() {
    return Util::GetKeys(creation_map_);
}

void Registry::AddType_(const Str &type_name, const std::type_info &info,
                        const CreationFunc &creation_func) {
    if (creation_map_.contains(type_name))
        throw Exception("Object type registered more than once: '" +
                        type_name + "'");
    creation_map_[type_name] = creation_func;
    type_name_map_[std::type_index(info)] = type_name;
}

Str Registry::FindTypeName_(const std::type_info &info) {
    auto it = type_name_map_.find(std::type_index(info));
    if (it == type_name_map_.end())
        throw Exception("Unknown object with typeid '" +
                        Str(info.name()) + "'");
    return it->second;
}

ObjectPtr Registry::CreateObjectOfType_(const Str &type_name, const Str &name,
                                        bool is_complete) {
    // Look up and call the CreationFunc.
    auto it = creation_map_.find(type_name);
    if (it == creation_map_.end())
        throw Exception("Unknown object type '" + type_name + "'");
    const CreationFunc &creation_func = it->second;

    // Call it, then tell the object to set up fields for parsing.
    ObjectPtr obj(creation_func());
    ASSERT(obj);
    obj->SetUp_(type_name, name, is_complete);
    return obj;
}

}  // namespace Parser
