#pragma once

#include <memory>

namespace Parser {

class Field;
class Object;

//! Convenience typedef for a shared_ptr to a Field.
typedef std::shared_ptr<Field>  FieldPtr;

//! Convenience typedef for a shared_ptr to an Object.
typedef std::shared_ptr<Object> ObjectPtr;

}  // namespace Parser
