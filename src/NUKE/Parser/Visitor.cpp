#include "Parser/Visitor.h"

#include <assert.h>

#include <vector>

#include "Parser/Field.h"
#include "Parser/Object.h"

namespace Parser {

void Visitor::VisitObjects(const Object &root, ObjectFunc func) {
    assert(func);

    func(root);

    // Recurse on objects in fields.
    for (const FieldPtr &field_ptr: root.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.type == ValueType::kObject) {
            const ObjectPtr obj = field.GetValue<ObjectPtr>();
            if (obj)
                VisitObjects(*obj, func);
        }
        else if (field.spec.type == ValueType::kObjectList) {
            const std::vector<ObjectPtr> objs =
                field.GetValue<std::vector<ObjectPtr>>();
            for (const ObjectPtr &obj: objs)
                if (obj)
                    VisitObjects(*obj, func);
        }
    }
}

}  // namespace Parser
