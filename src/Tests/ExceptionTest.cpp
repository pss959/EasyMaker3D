#include <string>

#include "Tests/Testing.h"
#include "Util/ExceptionBase.h"
#include "Util/FilePath.h"

class TestException : public ExceptionBase {
  public:
    explicit TestException(const std::string &msg) :
        ExceptionBase(msg) {}
    explicit TestException(const FilePath &path, const std::string &msg) :
        ExceptionBase(path, msg) {}
    explicit TestException(const FilePath &path, int line_number,
                           const std::string &msg) :
        ExceptionBase(path, line_number, msg) {}
};

TEST(ExceptionTest, TestException) {
    TestException ex("dummy message");
    EXPECT_EQ(std::string("dummy message"), std::string(ex.what()));
    EXPECT_LT(2U, ex.GetStackTrace().size());

    TestException pathex(FilePath("/a/b/c/d"), "another message");
    EXPECT_EQ(std::string("\n/a/b/c/d: another message"),
              std::string(pathex.what()));
    EXPECT_LT(2U, pathex.GetStackTrace().size());

    TestException pathlineex(FilePath("/a/b/c/d"), 123, "third message");
    EXPECT_EQ(std::string("\n/a/b/c/d:123: third message"),
              std::string(pathlineex.what()));
    EXPECT_LT(2U, pathlineex.GetStackTrace().size());
}
