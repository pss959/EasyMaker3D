#include <fstream>

#include "SG/FileMap.h"
#include "Tests/TempFile.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"
#include "Util/Delay.h"

class FileMapTest : public TestBase {};

TEST_F(FileMapTest, AddAndFind) {
    SG::FileMap fm;

    EXPECT_EQ("", fm.FindString("/no/such/path"));
    EXPECT_NULL(fm.FindImage("/no/such/image.jpg").Get());

    // Create paths to use for objects so that FileMap believes they exist. The
    // actual contents do not matter.
    TempFile tmp1("X");
    TempFile tmp2("X");

    fm.AddString(tmp1.GetPath(), "A test string");
    EXPECT_EQ("A test string", fm.FindString(tmp1.GetPath()));

    ion::gfx::ImagePtr image(new ion::gfx::Image);
    fm.AddImage(tmp2.GetPath(), image);
    EXPECT_EQ(image, fm.FindImage(tmp2.GetPath()));
}

TEST_F(FileMapTest, Modified) {
    SG::FileMap fm;

    // Same setup as above.
    TempFile tmp1("X");
    TempFile tmp2("X");

    fm.AddString(tmp1.GetPath(), "A test string");
    EXPECT_EQ("A test string", fm.FindString(tmp1.GetPath()));

    ion::gfx::ImagePtr image(new ion::gfx::Image);
    fm.AddImage(tmp2.GetPath(), image);
    EXPECT_EQ(image, fm.FindImage(tmp2.GetPath()));

    // Touch the files. Since the modification time is now newer, the contents
    // should be marked as invalid.
    Util::DelayThread(.01f); // Wait for times to be different enough.
    tmp1.SetContents("Y");
    tmp2.SetContents("Y");

    EXPECT_EQ("", fm.FindString(tmp1.GetPath()));
    EXPECT_NULL(fm.FindImage(tmp2.GetPath()).Get());
}

TEST_F(FileMapTest, Deleted) {
    SG::FileMap fm;

    // Save paths through TempFile destruction.
    FilePath path1;
    FilePath path2;

    // Same setup as above.
    {
        TempFile tmp1("X");
        TempFile tmp2("X");
        path1 = tmp1.GetPath();
        path2 = tmp1.GetPath();

        fm.AddString(path1, "A test string");
        EXPECT_EQ("A test string", fm.FindString(path1));

        ion::gfx::ImagePtr image(new ion::gfx::Image);
        fm.AddImage(path2, image);
        EXPECT_EQ(image, fm.FindImage(path2));
    }

    // The files are now deleted, so the contents should be marked as invalid.
    EXPECT_EQ("", fm.FindString(path1));
    EXPECT_NULL(fm.FindImage(path2).Get());
}

TEST_F(FileMapTest, Dependencies) {
    SG::FileMap fm;
    TempFile tmp1("X");
    TempFile tmp2("X");
    TempFile tmp3("X");
    fm.AddString(tmp1.GetPath(), "String 1");
    fm.AddString(tmp2.GetPath(), "String 2");
    fm.AddString(tmp3.GetPath(), "String 3");

    // tmp1 depends on tmp2 and tmp3.
    fm.AddDependency(tmp1.GetPath(), tmp2.GetPath());
    fm.AddDependency(tmp1.GetPath(), tmp3.GetPath());

    // All strings should be valid.
    EXPECT_EQ("String 1", fm.FindString(tmp1.GetPath()));
    EXPECT_EQ("String 2", fm.FindString(tmp2.GetPath()));
    EXPECT_EQ("String 3", fm.FindString(tmp3.GetPath()));

    // Update tmp2; tmp1 and tmp2 are no longer considered valid.
    Util::DelayThread(.01f); // Wait for times to be different enough.
    tmp2.SetContents("Y");
    EXPECT_EQ("",         fm.FindString(tmp1.GetPath()));
    EXPECT_EQ("",         fm.FindString(tmp2.GetPath()));
    EXPECT_EQ("String 3", fm.FindString(tmp3.GetPath()));
}

TEST_F(FileMapTest, UntrackedDependencies) {
    SG::FileMap fm;
    TempFile tmp1("X");
    TempFile tmp2("X");

    // tmp2 is untracked, causing tmp1 to be invalid.
    fm.AddString(tmp1.GetPath(), "String 1");
    fm.AddDependency(tmp1.GetPath(), tmp2.GetPath());
    EXPECT_EQ("", fm.FindString(tmp1.GetPath()));
}
