#include "Util/FilePath.h"

#if defined(ION_PLATFORM_WINDOWS)
#include <fileapi.h>
#endif

#include <algorithm>

#include <ion/port/environment.h>

#include "Util/Assert.h"

namespace Util {

/// Platform-dependent path construction.
static inline FilePath FromPath_(const std::filesystem::path &path) {
#if defined(ION_PLATFORM_WINDOWS)
    return FilePath(path.string());
#else
    return FilePath(path);
#endif
}

FilePath & FilePath::operator=(const char *path) {
    BaseType_::operator=(path);
    return *this;
}

FilePath & FilePath::operator=(const std::string &path) {
    BaseType_::operator=(path);
    return *this;
}

std::string FilePath::ToString() const {
    return generic_string();
}

std::string FilePath::ToNativeString() const {
#if defined(ION_PLATFORM_WINDOWS)
    return FilePath(*this).make_preferred().string();
#else
    return native();
#endif
}

bool FilePath::Exists() const {
    return std::filesystem::exists(*this);
}

bool FilePath::IsDirectory() const {
    return std::filesystem::is_directory(*this);
}

bool FilePath::IsAbsolute() const {
    return is_absolute();
}

bool FilePath::IsHidden() const {
#if defined(ION_PLATFORM_WINDOWS)
    return Exists() && (GetFileAttributes(ToWString(ToString()).c_str()) &
                        FILE_ATTRIBUTE_HIDDEN);
#else
    return Exists() && GetFileName()[0] == '.';
#endif
}

FilePath FilePath::GetParentDirectory() const {
    return FromPath_(parent_path());
}

std::string FilePath::GetFileName() const {
    return FromPath_(filename()).ToString();
}

std::string FilePath::GetExtension() const {
    return FromPath_(extension()).ToString();
}

FilePath FilePath::MakeRelativeTo(const FilePath &base_path) const {
    if (IsAbsolute())
        return *this;

    // If the base_path exists and is not known to be a directory, remove the
    // last component.
    else if (std::filesystem::exists(base_path) &&
             ! std::filesystem::is_directory(base_path))
        return FromPath_(base_path.parent_path() / *this);
    else
        return FromPath_(base_path / *this);
}

Time FilePath::GetModTime() const {
    ASSERT(Exists());
    return Time(std::filesystem::last_write_time(*this));
}

void FilePath::GetContents(std::vector<std::string> &subdirs,
                           std::vector<std::string> &files,
                           bool include_hidden) {
    subdirs.clear();
    files.clear();

    if (! IsDirectory())
        return;

    for (const auto &entry: std::filesystem::directory_iterator(*this)) {
        if (! include_hidden && FromPath_(entry.path()).IsHidden())
            continue;

        const auto name = FromPath_(entry.path().filename()).ToString();

        if (entry.is_directory()) {
            // Filter out "." and "..".
            if (name != "." && name != "..")
                subdirs.push_back(name);
        }
        else if (entry.is_regular_file())
            files.push_back(name);
    }

    std::sort(subdirs.begin(), subdirs.end());
    std::sort(files.begin(),   files.end());
}

void FilePath::Remove() {
    ASSERT(Exists());
    std::filesystem::remove(*this);

}

FilePath FilePath::Join(const FilePath &p0, const FilePath &p1) {
    ASSERT(! p1.IsAbsolute());
    FilePath result = p0;
    result /= p1;
    return result;
}

FilePath FilePath::GetResourceBasePath() {
    return FilePath(RESOURCE_DIR);
}

FilePath FilePath::GetResourcePath(const std::string &type_name,
                                   const FilePath &sub_path) {
    FilePath path = GetResourceBasePath();
    path /= type_name;
    path /= sub_path;
    return path;
}

FilePath FilePath::GetFullResourcePath(const std::string &subdir,
                                       const FilePath &path) {
    if (path.IsAbsolute())
        return path;
    else
        return GetResourcePath(subdir, path);
}

FilePath FilePath::GetHomeDirPath() {
    const FilePath dir = ion::port::GetEnvironmentVariableValue("HOME");
    ASSERT(dir.Exists());
    return dir;
}

FilePath FilePath::GetSettingsDirPath() {
#if defined(ION_PLATFORM_WINDOWS)
    FilePath dir = ion::port::GetEnvironmentVariableValue("APPDATA");
#else
    FilePath dir = ion::port::GetEnvironmentVariableValue("HOME");
#endif
    ASSERT(std::filesystem::is_directory(dir));

    dir /= "imakervr";
    return dir;
}

FilePath FilePath::GetTestDataPath() {
    return FilePath(TEST_DATA_DIR);
}

std::string FilePath::GetSeparator() {
#if defined(ION_PLATFORM_WINDOWS)
    return FromWString(std::wstring(1, preferred_separator));
#else
    return std::string(1, preferred_separator);
#endif
}

}  // namespace Util
