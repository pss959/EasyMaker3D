#include "Parser/Registry.h"

#include "SG/Exception.h"
#include "Util/General.h"

namespace Parser {

std::unordered_map<std::string, Registry::CreationFunc> Registry::map_;

void Registry::AddType_(const std::string &type_name,
                        const CreationFunc &creation_func) {
    if (Util::MapContains(map_, type_name))
        throw Exception("Object type registered more than once: '" +
                        type_name + "'");
    map_[type_name] = creation_func;
}

ObjectPtr Registry::CreateObjectOfType(const std::string &type_name) {
    // Look up and call the CreationFunc.
    auto it = map_.find(type_name);
    if (it == map_.end())
        throw Exception("Unknown object type '" + type_name + "'");
    const CreationFunc &creation_func = it->second;

    // Call it, then tell the object to set up fields for parsing.
    ObjectPtr obj(creation_func());
    ASSERT(obj);
    obj->SetTypeName(type_name);
    obj->AddFields();
    return obj;
}

std::vector<std::string> Registry::GetAllTypeNames() {
    std::vector<std::string> keys;
    keys.reserve(map_.size());
    std::transform(map_.begin(), map_.end(), std::back_inserter(keys),
                   [](auto &pair){ return pair.first; });
    return keys;
}

}  // namespace Parser
