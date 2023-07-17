#include "Tests/Testing.h"
#include "Util/FilePathList.h"

/// Derived FilePathList class that simulates a file system for testing.
class TestFPL_ : public FilePathList {
  public:
    virtual bool IsValidDirectory(const FilePath &path) const {
        const std::string fn = path.GetFileName();
        return fn.contains("DIR");
    }
    virtual bool IsExistingFile(const FilePath &path) const {
        return path.GetFileName().contains("FILE");
    }
};

TEST(FilePathList, Basics) {
    TestFPL_ fpl;
    fpl.Init(FilePath("/abc/def"));
    EXPECT_EQ("/abc/def", fpl.GetCurrent().ToString());
    EXPECT_FALSE(fpl.IsValidDirectory("/abc/def"));
    EXPECT_TRUE(fpl.IsValidDirectory("/abc/DIR1"));
    EXPECT_TRUE(fpl.IsExistingFile("/abc/def/FILE"));
    EXPECT_FALSE(fpl.IsExistingFile("/abc/def/blah"));

    fpl.AddPath(FilePath("/ghi/jkl/ANOTHERFILE"));
    EXPECT_EQ("/ghi/jkl/ANOTHERFILE", fpl.GetCurrent().ToString());
    EXPECT_FALSE(fpl.IsValidDirectory(fpl.GetCurrent()));
    EXPECT_TRUE(fpl.IsExistingFile(fpl.GetCurrent()));
}

TEST(FilePathList, Directions) {
    TestFPL_ fpl;
    fpl.Init(FilePath("/DIR0/DIR1"));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kUp));
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kForward));
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kBack));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kHome));

    fpl.GoInDirection(FilePathList::Direction::kUp);
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kUp));
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kForward));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kBack));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kHome));

    fpl.AddPath(FilePath("/DIR0/AFILE"));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kUp));
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kForward));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kBack));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kHome));

    fpl.GoInDirection(FilePathList::Direction::kBack);
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kUp));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kForward));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kBack));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kHome));

    fpl.GoInDirection(FilePathList::Direction::kForward);
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kUp));
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kForward));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kBack));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kHome));

    fpl.GoInDirection(FilePathList::Direction::kHome);
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kUp));
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kForward));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kBack));
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kHome));
}
