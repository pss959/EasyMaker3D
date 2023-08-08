#include "Parser/Registry.h"
#include "Tests/Parser/ParserTestBase.h"
#include "Util/General.h"

class RegistryTest : public ParserTestBase {};

TEST_F(RegistryTest, GetAllTypeNames) {
    // No names by default.
    Parser::Registry::Clear();
    EXPECT_TRUE(Parser::Registry::GetAllTypeNames().empty());

    InitTestClasses();
    const auto names = Parser::Registry::GetAllTypeNames();

    // Test expected names.
    EXPECT_EQ(5U, names.size());
    EXPECT_TRUE(Util::Contains(names, "Simple"));
    EXPECT_TRUE(Util::Contains(names, "Derived"));
    EXPECT_TRUE(Util::Contains(names, "Full"));
    EXPECT_TRUE(Util::Contains(names, "Other"));
    EXPECT_TRUE(Util::Contains(names, "Unscoped"));

    Parser::Registry::Clear();
    EXPECT_TRUE(Parser::Registry::GetAllTypeNames().empty());
}

TEST_F(RegistryTest, CreateObjectOfType) {
    Parser::ObjectPtr obj = Parser::Registry::CreateObjectOfType("Derived");
    EXPECT_EQ("Derived", obj->GetTypeName());
    EXPECT_TRUE(Util::IsA<Derived>(obj));

    obj = Parser::Registry::CreateObjectOfType("Other");
    EXPECT_EQ("Other", obj->GetTypeName());
    EXPECT_TRUE(Util::IsA<Other>(obj));

    // Cannot create unregistered class.
    TEST_THROW(Parser::Registry::CreateObjectOfType("Blahhhh"),
               Parser::Exception, "Unknown object type");
}

TEST_F(RegistryTest, CreateObject) {
    EXPECT_TRUE(Parser::Registry::CreateObject<Derived>());

    // Cannot create unknown class.
    Parser::Registry::Clear();
    TEST_THROW(Parser::Registry::CreateObject<Derived>(),
               Parser::Exception, "Unknown object with typeid");
}
