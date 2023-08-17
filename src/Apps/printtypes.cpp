#include <iostream>
#include <string>
#include <vector>

#include "App/RegisterTypes.h"
#include "Models/Model.h"
#include "Parser/Field.h"
#include "Parser/Object.h"
#include "Parser/Registry.h"
#include "Util/Assert.h"

/// \file
/// The printtypes application writes to standard output all known concrete
/// parsable types with their fields.
///
/// \ingroup Apps

int main() {
    // Register all known concrete types with the Parser::Registry.
    RegisterTypes();

    // Get the names of all types and sort them.
    StrVec names = Parser::Registry::GetAllTypeNames();
    std::sort(names.begin(), names.end());

    for (auto &name: names) {
        // Pass false for is_complete because some types require context.
        Parser::ObjectPtr obj =
            Parser::Registry::CreateObjectOfType(name, "", false);
        ASSERT(obj);
        const auto &fields = obj->GetFields();

        StrVec field_names;
        for (const auto &field: fields)
            field_names.push_back(field->GetName());
        std::sort(field_names.begin(), field_names.end());

        std::cout << name << ":\n";
        for (const auto &fn: field_names)
            std::cout << "   " << fn << "\n";

        // Special case for Model classes.
        if (const auto model = std::dynamic_pointer_cast<Model>(obj))
            std::cout << "   " << model->GetTypeName()
                      << " Can set complexity: "
                      << (model->CanSetComplexity() ? "Yes" : "No") << "\n";

        std::cout << "\n";
    }

    return 0;
}

