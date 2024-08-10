#include "Commands/CreateExtrudedModelCommand.h"
#include "Commands/CreateImportedModelCommand.h"
#include "Commands/CreatePrimitiveModelCommand.h"
#include "Commands/CreateRevSurfModelCommand.h"
#include "Commands/CreateTextModelCommand.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// This tests the base CreateModelCommand and all classes derived from it.
/// \ingroup Tests
class CreateModelCommandTest : public CommandTestBase {};

TEST_F(CreateModelCommandTest, CreateModelCommand) {
    // Need to use a derived concrete class here.
    auto cemc = Command::CreateCommand<CreateExtrudedModelCommand>();
    EXPECT_TRUE(cemc->GetResultName().empty());
    EXPECT_EQ(1,                 cemc->GetInitialScale());
    EXPECT_EQ(Point3f::Zero(),   cemc->GetTargetPosition());
    EXPECT_EQ(Vector3f::AxisY(), cemc->GetTargetDirection());

    SetParseTypeName("CreateExtrudedModelCommand");
    cemc->SetResultName("Alvin");
    cemc->SetInitialScale(4.5f);
    cemc->SetTargetPosition(Point3f(1, 2, 3));
    cemc->SetTargetDirection(Vector3f(1, 0, 0));
    EXPECT_EQ("Alvin",           cemc->GetResultName());
    EXPECT_EQ(4.5f,              cemc->GetInitialScale());
    EXPECT_EQ(Point3f(1, 2, 3),  cemc->GetTargetPosition());
    EXPECT_EQ(Vector3f(1, 0, 0), cemc->GetTargetDirection());
}

TEST_F(CreateModelCommandTest, CreateExtrudedModelCommand) {
    SetParseTypeName("CreateExtrudedModelCommand");
    TestInvalid("", "Invalid model name");
    TestInvalid(R"(result_name: " BadName")", "Invalid model name");
    TestValid(R"(result_name: "Xyz")");
    TestDesc(R"(result_name: "Bob")", R"(Created extruded Model "Bob")");
}

TEST_F(CreateModelCommandTest, CreateImportedModelCommand) {
    auto cimc = Command::CreateCommand<CreateImportedModelCommand>();
    EXPECT_TRUE(cimc->GetPath().empty());
    cimc->SetPath("/a/b/c.stl");
    EXPECT_EQ("/a/b/c.stl", cimc->GetPath());

    SetParseTypeName("CreateImportedModelCommand");
    TestInvalid("", "Invalid model name");
    TestInvalid(R"(result_name: " BadName")", "Invalid model name");
    TestValid(R"(result_name: "Xyz")");
    TestDesc(R"(result_name: "Bob", path: "/a/b/c.stl")",
             R"(Created imported Model "Bob")");
}

TEST_F(CreateModelCommandTest, CreatePrimitiveModelCommand) {
    auto cpmc = Command::CreateCommand<CreatePrimitiveModelCommand>();
    EXPECT_ENUM_EQ(PrimitiveType::kBox, cpmc->GetType());
    cpmc->SetType(PrimitiveType::kCylinder);
    EXPECT_ENUM_EQ(PrimitiveType::kCylinder, cpmc->GetType());

    SetParseTypeName("CreatePrimitiveModelCommand");
    TestInvalid("", "Invalid model name");
    TestInvalid(R"(result_name: " BadName")", "Invalid model name");
    TestValid(R"(result_name: "Xyz")");
    TestDesc(R"(result_name: "Bob", type: "kBox")",
             R"(Created Box Model "Bob")");
    TestDesc(R"(result_name: "Sue", type: "kSphere")",
             R"(Created Sphere Model "Sue")");
}

TEST_F(CreateModelCommandTest, CreateRevSurfModelCommand) {
    SetParseTypeName("CreateRevSurfModelCommand");
    TestInvalid("", "Invalid model name");
    TestInvalid(R"(result_name: " BadName")", "Invalid model name");
    TestValid(R"(result_name: "Xyz")");
    TestDesc(R"(result_name: "Bob")",
             R"(Created surface of revolution Model "Bob")");
}

TEST_F(CreateModelCommandTest, CreateTextModelCommand) {
    auto ctmc = Command::CreateCommand<CreateTextModelCommand>();
    EXPECT_EQ("A", ctmc->GetText());
    ctmc->SetText("Hello");
    EXPECT_EQ("Hello", ctmc->GetText());

    SetParseTypeName("CreateTextModelCommand");
    TestInvalid("", "Invalid model name");
    TestInvalid(R"(result_name: " BadName")", "Invalid model name");
    TestValid(R"(result_name: "Xyz")");
    TestDesc(R"(result_name: "Bob")", R"(Created 3D text Model "Bob")");
}
