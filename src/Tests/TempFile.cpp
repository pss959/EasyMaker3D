#include "Tests/TempFile.h"

#include <algorithm>
#include <fstream>
#include <random>

#include "Util/Assert.h"
#include "Util/FileSystem.h"
#include "Util/Read.h"

TempFile::TempFile(const Str &input_string) {
    // Generate a random file name. This is used because: std::filesystem has
    // no current way of generating a unique file name, std::tmpnam() is marked
    // as "dangerous", std::tmpfile() has no good way of returning the file
    // name, and I don't want to depend on boost::filesystem. This was stolen
    // from the internet.
    const Str name =
        RandomString_(4) + "-" + RandomString_(4) + "-" + RandomString_(4);

    path_ = FilePath::Join(FilePath::GetTempFilePath(), FilePath(name));

    std::ofstream out(path_.ToNativeString());
    ASSERTM(out.is_open(), "Could not open temp file: " + path_.ToString());
    out << input_string;
    out.close();
}

TempFile::~TempFile() {
    // Have to use the real FileSystem for this.
    FileSystem fs;
    fs.Remove(path_.ToString());
}

Str TempFile::GetContents() const {
    Str s;
    if (! Util::ReadFile(path_, s))
        ASSERTM(false, "Cannot read from TempFile " + path_.ToString());
    return s;
}

void TempFile::SetContents(const Str &new_contents) {
    std::ofstream out(path_.ToNativeString());
    ASSERT(out.is_open());
    out << new_contents;
    out.close();
}

Str TempFile::RandomString_(size_t length) {
    auto random_char = []() -> char {
        const char alphanumeric[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
            ;
        return alphanumeric[rand() % (sizeof(alphanumeric) - 1)];
    };
    Str str(length, '\0');
    std::generate_n(str.begin(), length, random_char);
    return str;
}
