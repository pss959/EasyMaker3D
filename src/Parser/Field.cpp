#include "Parser/Field.h"

namespace Parser {

// Instantiate ParseValue() function for supported types.

template <> void TField<bool>::ParseValue(Scanner &scanner) {
    value_ = scanner.ScanBool();
}

template <> void TField<int>::ParseValue(Scanner &scanner) {
    value_ = scanner.ScanInteger();
}

template <> void TField<unsigned int>::ParseValue(Scanner &scanner) {
    value_ = scanner.ScanUInteger();
}

template <> void TField<std::string>::ParseValue(Scanner &scanner) {
    value_ = scanner.ScanQuotedString();
}

template <> void TField<float>::ParseValue(Scanner &scanner) {
    value_ = scanner.ScanFloat();
}

}  // namespace Parser
