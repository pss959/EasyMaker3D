#include "App/RegisterTypes.h"
#include "Items/Shelf.h"
#include "Parser/Registry.h"
#include "SG/Shape.h"
#include "SG/TextNode.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"
#include "Util/General.h"

class RegistryTest : public TestBase {
  protected:
    RegistryTest() {
        RegisterTypes();
    }
    ~RegistryTest() {
        UnregisterTypes();
    }
};

TEST_F(RegistryTest, CreateObjectOfType) {
    Parser::ObjectPtr obj = Parser::Registry::CreateObjectOfType("TextNode");
    EXPECT_EQ("TextNode", obj->GetTypeName());
    EXPECT_TRUE(Util::IsA<SG::TextNode>(obj));

    obj = Parser::Registry::CreateObjectOfType("Shelf");
    EXPECT_EQ("Shelf", obj->GetTypeName());
    EXPECT_TRUE(Util::IsA<Shelf>(obj));

    // Cannot create unknown or abstract class.
    TEST_THROW(Parser::Registry::CreateObjectOfType("Blahhhh"),
               Parser::Exception, "Unknown object type");
    TEST_THROW(Parser::Registry::CreateObjectOfType("SG::Shape"),
               Parser::Exception, "Unknown object type");
}

TEST_F(RegistryTest, CreateObject) {
    EXPECT_TRUE(Parser::Registry::CreateObject<SG::TextNode>());

    // Cannot create abstract class.
    TEST_THROW(Parser::Registry::CreateObject<SG::Shape>(),
               Parser::Exception, "Unknown object with typeid");
}
