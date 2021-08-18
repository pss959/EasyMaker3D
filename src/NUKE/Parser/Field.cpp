#include "Field.h"

#include "Parser/ArrayField.h"
#include "Parser/Exception.h"
#include "Parser/SingleField.h"
#include "Util/String.h"

namespace Parser {

template <typename T>
const T & Field::GetValue() const {
    if (spec.count > 1) {
        throw Exception(std::string("Attempt to GetValue() for field '") +
                        spec.name + "' with count " +
                        Util::ToString(spec.count));
    }
    try {
        return std::get<T>(((SingleField_ *) this)->value);
    }
    catch (std::bad_variant_access &ex) {
        throw Exception(
            std::string("Invalid type for GetValue() for field '") +
                        spec.name + "'");
    }
}

template <typename T>
std::vector<T> Field::GetValues() const {
    if (spec.count == 1) {
        throw Exception(std::string("Attempt to GetValues() for field '") +
                        spec.name + "' with count 1");
    }
    try {
        std::vector<T> values;
        values.reserve(spec.count);
        for (uint32_t i = 0; i < spec.count; ++i)
            values.push_back(std::get<T>(((ArrayField_ *) this)->values[i]));
        return values;
    }
    catch (std::bad_variant_access &ex) {
        throw Exception(
            std::string("Invalid type for GetValues() for field '") +
                        spec.name + "'");
    }
}

// Explicitly instantiate functions for known types.
#define INSTANTIATE_GET_VALUE_(TYPE) \
    template const TYPE &      Field::GetValue<TYPE>() const
#define INSTANTIATE_GET_VALUES_(TYPE) \
    template std::vector<TYPE> Field::GetValues<TYPE>() const

INSTANTIATE_GET_VALUE_(bool);
INSTANTIATE_GET_VALUE_(int);
INSTANTIATE_GET_VALUE_(unsigned int);
INSTANTIATE_GET_VALUE_(float);
INSTANTIATE_GET_VALUE_(std::string);
INSTANTIATE_GET_VALUE_(ObjectPtr);
INSTANTIATE_GET_VALUE_(std::vector<ObjectPtr>);

INSTANTIATE_GET_VALUES_(bool);
INSTANTIATE_GET_VALUES_(int);
INSTANTIATE_GET_VALUES_(unsigned int);
INSTANTIATE_GET_VALUES_(float);
INSTANTIATE_GET_VALUES_(std::string);

#undef INSTANTIATE_GET_VALUE_
#undef INSTANTIATE_GET_VALUES_

}  // namespace Parser
