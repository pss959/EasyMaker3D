#include "Tests/TestBaseWithTypes.h"

#include "gmock/gmock.h"

#include "App/RegisterTypes.h"
#include "Parser/Exception.h"
#include "Parser/Parser.h"
#include "Tests/Testing.h"
#include "Util/Assert.h"

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

void TestBaseWithTypes::TestValid(const std::string &contents) {
    const std::string str = BuildParseString(contents);
    try {
        auto obj = ParseObject(str);
        EXPECT_NOT_NULL(obj);
    }
    catch (std::exception &ex) {
        EXPECT_TRUE(false) << "Parsing '" << str
                           << "' failed with exception " << ex.what();
    }
}

void TestBaseWithTypes::TestInvalid(const std::string &contents,
                                    const std::string &error) {
    // Don't use TEST_THROW here; this creates a better message on failure.
    const std::string str = BuildParseString(contents);
    EXPECT_THROW({
            try {
                ParseObject(str);
            }
            catch (const Parser::Exception &ex) {
                EXPECT_THAT(ex.what(), ::testing::HasSubstr(error))
                    << "\nFor input: '" << str << "'";
                throw;
            }
        }, Parser::Exception);
}

std::string TestBaseWithTypes::BuildParseString(
    const std::string &contents) const {
    ASSERTM(! parse_type_name_.empty(), "SetParseTypeName() was not called");
    return parse_type_name_ + " { " + contents + " }";
}
