#include <string>

#include "Tests/Testing.h"
#include "Parser/Exception.h"
#include "Util/FilePath.h"

TEST(ExceptionTest, Constructors) {
    Parser::Exception ex1("dummy message");
    EXPECT_EQ(std::string("dummy message"), std::string(ex1.what()));

    Parser::Exception ex2(FilePath("/a/b/c/d"), "another message");
    EXPECT_EQ(std::string("\n/a/b/c/d: Parse error: another message"),
              std::string(ex2.what()));

    Parser::Exception ex3(FilePath("/a/b/c/d"), 123, "third message");
    EXPECT_EQ(std::string("\n/a/b/c/d:123: Parse error: third message"),
              std::string(ex3.what()));
}
