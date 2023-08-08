#include "Tests/TempFile.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"
#include "Util/Read.h"
#include "Util/String.h"

/// \ingroup Tests
class ReadTest : public TestBase {};

TEST_F(ReadTest, NoIncludes) {
    const std::string contents =
        "This is test input for Util::ReadFile().\n"
        "\n"
        "   @include \"IncludeThis.txt\"\n"
        "\n"
        "That's all!\n";
    TempFile tmp_file(contents);
    std::string s;
    EXPECT_TRUE(Util::ReadFile(tmp_file.GetPath(), s, false));
    EXPECT_EQ(contents, FixString(s));
}

TEST_F(ReadTest, YesIncludes) {
    // Contents of included file.
    const std::string included =
        "Some text.\n"
        "Some more text.\n";

    // Contents of read file. The <INCLUDED> is replaced with the path to the
    // included temp file.
    const std::string contents =
        "This is test input for Util::ReadFile().\n"
        "\n"
        "   @include \"<INCLUDED>\"\n"
        "\n"
        "That's all!\n";
    const std::string expected = Util::ReplaceString(
        contents, "@include \"<INCLUDED>\"", included);

    TempFile tmp_inc(included);
    TempFile tmp_file(Util::ReplaceString(contents, "<INCLUDED>",
                                          tmp_inc.GetPath().ToString()));

    std::string s;
    EXPECT_TRUE(Util::ReadFile(tmp_file.GetPath(), s, true));
    EXPECT_EQ(expected, FixString(s));
}

TEST_F(ReadTest, IncludeErrors) {
    {
        // No open quote.
        TempFile tmp_file("  @include  \n");
        std::string s;
        EXPECT_FALSE(Util::ReadFile(tmp_file.GetPath(), s, true));
    }
    {
        // Bad stuff after @include.
        TempFile tmp_file("  @include somefile \n");
        std::string s;
        EXPECT_FALSE(Util::ReadFile(tmp_file.GetPath(), s, true));
    }
    {
        // No close quote.
        TempFile tmp_file("  @include \"somefile\n");
        std::string s;
        EXPECT_FALSE(Util::ReadFile(tmp_file.GetPath(), s, true));
    }
    {
        // No such file to include.
        TempFile tmp_file("  @include \"/file/does/not/exist.txt\"\n");
        std::string s;
        EXPECT_FALSE(Util::ReadFile(tmp_file.GetPath(), s, true));
    }
}

TEST_F(ReadTest, ReadImage) {
    auto image = Util::ReadImage(GetDataPath("testimage.jpg"), false);
    EXPECT_NOT_NULL(image.Get());
    EXPECT_EQ(154U, image->GetWidth());
    EXPECT_EQ(148U, image->GetHeight());
}

TEST_F(ReadTest, ReadBadImage) {
    auto image = Util::ReadImage(GetDataPath("NOSUCHIMAGE.jpg"), false);
    EXPECT_NULL(image.Get());
}

TEST_F(ReadTest, ReadShape) {
    // Read a shape in each supported format.
    const std::vector<std::string> extensions{
        "3ds", "dae", "lwo", "obj", "off"
    };
    for (const auto &ext: extensions) {
        auto read_it = [&](bool use_normals, bool use_tex_coords){
            auto shape = Util::ReadShape(GetDataPath("Shapes/shape." + ext),
                                         use_normals, use_tex_coords);
            EXPECT_NOT_NULL(shape.Get());
        };
        // Try all combinations of using normals and texture coordinates.
        read_it(false, false);
        read_it(false, true);
        read_it(true,  false);
        read_it(true,  true);
    }
}

TEST_F(ReadTest, ReadBadShape) {
    auto shape = Util::ReadShape(GetDataPath("NOSUCHSHAPE.off"), false, false);
    EXPECT_NULL(shape.Get());
}

