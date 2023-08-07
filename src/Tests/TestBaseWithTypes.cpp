#include "Tests/TestBaseWithTypes.h"

#include "App/RegisterTypes.h"
#include "Parser/Parser.h"

TestBaseWithTypes::TestBaseWithTypes() {
    RegisterTypes();
}

TestBaseWithTypes::~TestBaseWithTypes() {
    UnregisterTypes();
}

Parser::ObjectPtr TestBaseWithTypes::ParseItem_(const std::string &input) {
    Parser::Parser parser;
    return parser.ParseFromString(input);
}
