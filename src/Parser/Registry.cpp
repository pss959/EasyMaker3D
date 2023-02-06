#include "Parser/Registry.h"

#include "Parser/Exception.h"
#include "Util/General.h"

namespace Parser {

Registry::TypeNameMap_ Registry::type_name_map_;
Registry::CreationMap_ Registry::creation_map_;

std::vector<std::string> Registry::GetAllTypeNames() {
    return Util::GetKeys(creation_map_);
}

void Registry::AddType_(const std::string &type_name,
                        const std::type_info &info,
                        const CreationFunc &creation_func) {
    if (Util::MapContains(creation_map_, type_name))
        throw Exception("Object type registered more than once: '" +
                        type_name + "'");
    creation_map_[type_name] = creation_func;
    type_name_map_[std::type_index(info)] = type_name;
}

std::string Registry::FindTypeName_(const std::type_info &info) {
    auto it = type_name_map_.find(std::type_index(info));
    if (it == type_name_map_.end())
        throw Exception("Unknown object with typeid '" +
                        std::string(info.name()) + "'");
    return it->second;
}

ObjectPtr Registry::CreateObjectOfType_(const std::string &type_name,
                                        const std::string &name,
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
