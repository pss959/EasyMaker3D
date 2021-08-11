#pragma once

namespace Parser {

//! Types of values that can be stored in a parsed Field. Note that the
//! ValueType::kObject and ValueType::kObjectList types cannot be used in
//! fields with count > 1.
//! \ingroup Parser
enum class ValueType {
    //! A boolean value, parsed as "true", "t", "false", or "f"
    //! (case-insensitive).
    kBool,
    //! A signed integer value.
    kInteger,
    //! An unsigned integer value.
    kUInteger,
    //! A floating-point value.
    kFloat,
    //! A double-quoted strings. Note that escape sequences are not yet
    //! supported.
    kString,
    //! A nested Object.
    kObject,
    //! A nested collection of zero or more Objects. The collection is
    //! surrounded by square brackets and Object in it are separated by commas.
    kObjectList,
};

}  // namespace Parser
