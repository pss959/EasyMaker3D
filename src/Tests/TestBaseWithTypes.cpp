#include "Tests/TestBaseWithTypes.h"

#include "App/RegisterTypes.h"
#include "Parser/Exception.h"
#include "Parser/Parser.h"
#include "Tests/Testing.h"

TestBaseWithTypes::TestBaseWithTypes() {
    RegisterTypes();
}

TestBaseWithTypes::~TestBaseWithTypes() {
    UnregisterTypes();
}

Parser::ObjectPtr TestBaseWithTypes::ParseObject(const std::string &input) {
    Parser::Parser parser;
    return parser.ParseFromString(input);
}

void TestBaseWithTypes::TestValid(const std::string &str) {
    auto obj = ParseObject(str);
    EXPECT_NOT_NULL(obj);
}

void TestBaseWithTypes::TestInvalid(const std::string &str,
                                    const std::string &error) {
    TEST_THROW(ParseObject(str), Parser::Exception, error);
}

