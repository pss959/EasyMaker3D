//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/TestBaseWithTypes.h"

#include "gmock/gmock.h"

#include "App/RegisterTypes.h"
#include "Parser/Exception.h"
#include "Parser/Parser.h"
#include "Tests/Testing2.h"
#include "Util/Assert.h"

TestBaseWithTypes::TestBaseWithTypes() {
    RegisterTypes();

    // Test that calling it again does not hurt.
    RegisterTypes();
}

TestBaseWithTypes::~TestBaseWithTypes() {
    UnregisterTypes();
}

Parser::ObjectPtr TestBaseWithTypes::ParseObject(const Str &input) {
    Parser::Parser parser;
    return parser.ParseFromString(input);
}

void TestBaseWithTypes::TestValid(const Str &contents) {
    const Str str = BuildParseString(contents);
    try {
        auto obj = ParseObject(str);
        EXPECT_NOT_NULL(obj);
    }
    catch (std::exception &ex) {
        EXPECT_TRUE(false) << "Parsing '" << str
                           << "' failed with exception " << ex.what();
    }
}

void TestBaseWithTypes::TestInvalid(const Str &contents, const Str &error) {
    // Don't use TEST_THROW here; this creates a better message on failure.
    const Str str = BuildParseString(contents);
    EXPECT_THROW({
            try {
                ParseObject(str);
            }
            catch (const Parser::Exception &ex) {
                EXPECT_STR_HAS(ex.what(), error)
                    << "\nFor input: '" << str << "'";
                throw;
            }
        }, Parser::Exception);
}

Str TestBaseWithTypes::BuildParseString(const Str &contents) const {
    ASSERTM(! parse_type_name_.empty(), "SetParseTypeName() was not called");
    return parse_type_name_ + R"("Name" { )" + contents + " }";
}
