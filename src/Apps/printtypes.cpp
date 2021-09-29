#include <iostream>
#include <string>
#include <vector>

#include "Assert.h"
#include "Parser/Field.h"
#include "Parser/Object.h"
#include "Parser/Registry.h"
#include "RegisterTypes.h"

int main() {
    // Register all known concrete types with the Parser::Registry.
    RegisterTypes();

    // Get the names of all types and sort them.
    std::vector<std::string> names = Parser::Registry::GetAllTypeNames();
    std::sort(names.begin(), names.end());

    for (auto &name: names) {
        Parser::ObjectPtr obj = Parser::Registry::CreateObjectOfType(name);
        ASSERT(obj);
        const auto &fields = obj->GetFields();

        std::vector<std::string> field_names;
        for (const auto &field: fields)
            field_names.push_back(field->GetName());
        std::sort(field_names.begin(), field_names.end());

        std::cout << name << ":\n";
        for (const auto &fn: field_names)
            std::cout << "   " << fn << "\n";
        std::cout << "\n";
    }

    return 0;
}

