#include <string>

#include "Tests/Testing.h"
#include "Util/ExceptionBase.h"
#include "Util/FilePath.h"

/// \ingroup Tests
class TestException : public ExceptionBase {
  public:
    explicit TestException(const Str &msg) :
        ExceptionBase(msg) {}
    explicit TestException(const FilePath &path, const Str &msg) :
        ExceptionBase(path, msg) {}
    explicit TestException(const FilePath &path, int line_number,
                           const Str &msg) :
        ExceptionBase(path, line_number, msg) {}
};

TEST(ExceptionTest, TestException) {
    TestException ex("dummy message");
    EXPECT_EQ(Str("dummy message"), Str(ex.what()));
    EXPECT_LT(2U, ex.GetStackTrace().size());

    TestException pathex(FilePath("/a/b/c/d"), "another message");
    EXPECT_EQ(Str("\n/a/b/c/d: another message"), Str(pathex.what()));
    EXPECT_LT(2U, pathex.GetStackTrace().size());

    TestException pathlineex(FilePath("/a/b/c/d"), 123, "third message");
    EXPECT_EQ(Str("\n/a/b/c/d:123: third message"), Str(pathlineex.what()));
    EXPECT_LT(2U, pathlineex.GetStackTrace().size());
}
