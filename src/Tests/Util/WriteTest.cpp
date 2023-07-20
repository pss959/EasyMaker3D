#include "Tests/TestBase.h"
#include "Tests/Testing.h"
#include "Util/Read.h"
#include "Util/Write.h"

class WriteTest : public TestBase {};

TEST_F(WriteTest, WriteString) {
    // Create a temporary file to get a path. The contents will be overwritten
    // by Util::WriteString().
    TempFile tmp_file("Wrong contents\n");
    EXPECT_TRUE(Util::WriteString(tmp_file.GetPath(), "New contents\n"));
    std::string s;
    EXPECT_TRUE(Util::ReadFile(tmp_file.GetPath(), s));
    EXPECT_EQ("New contents\n", s);

    // Fail on unknown file.
    EXPECT_FALSE(Util::WriteString("/no/such/file.txt", "New contents\n"));
}

TEST_F(WriteTest, WriteImage) {
    auto image = Util::ReadImage(GetDataPath("testimage.jpg"), false);
    EXPECT_NOT_NULL(image.Get());

    TempFile tmp_file("");
    EXPECT_TRUE(Util::WriteImage(tmp_file.GetPath(), *image, false));

    // Read the result and compare images. Note that comparing data exactly
    // may not work because the jpg data will likely differ.
    auto copy = Util::ReadImage(tmp_file.GetPath(), false);
    EXPECT_NOT_NULL(copy.Get());

    EXPECT_EQ(image->GetFormat(),     copy->GetFormat());
    EXPECT_EQ(image->GetType(),       copy->GetType());
    EXPECT_EQ(image->GetDimensions(), copy->GetDimensions());
    EXPECT_EQ(image->GetWidth(),      copy->GetWidth());
    EXPECT_EQ(image->GetHeight(),     copy->GetHeight());
    EXPECT_EQ(image->GetDepth(),      copy->GetDepth());
    EXPECT_EQ(image->GetDataSize(),   copy->GetDataSize());
}
