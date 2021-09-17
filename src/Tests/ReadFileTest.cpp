#include "Testing.h"
#include "Util/FilePath.h"
#include "Util/Read.h"

class ReadFileTest : public TestBase {};

TEST_F(ReadFileTest, NoIncludes) {
    Util::FilePath path = GetDataPath("ReadThis.txt");
    const std::string expected =
        "This is a test input file for Util::ReadFile().\n"
        "\n"
        "   @include \"IncludeThis.txt\"\n"
        "\n"
        "That's all!\n";
    std::string s;
    EXPECT_TRUE(Util::ReadFile(path, s, false));
    EXPECT_EQ(expected, s);
}

TEST_F(ReadFileTest, YesIncludes) {
    Util::FilePath path = GetDataPath("ReadThis.txt");
    const std::string expected =
        "This is a test input file for Util::ReadFile().\n"
        "\n"
        "   Some text.\n"
        "Some more text.\n"
        "\n"
        "\n"
        "That's all!\n";
    std::string s;
    EXPECT_TRUE(Util::ReadFile(path, s, true));
    EXPECT_EQ(expected, s);
}
