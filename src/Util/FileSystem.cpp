#include "Util/FileSystem.h"

#ifdef ION_PLATFORM_WINDOWS
#include <fileapi.h>
#endif

#include <algorithm>

#include <ion/port/environment.h>

#include "Util/Assert.h"
#include "Util/String.h"

FileSystemPtr FileSystem::real_file_system_(new FileSystem);
FileSystemPtr FileSystem::cur_file_system_ = real_file_system_;

void FileSystem::Install(const FileSystemPtr &fs) {
    cur_file_system_ = fs ? fs : real_file_system_;
}

FileSystemPtr FileSystem::GetInstalled() {
    return cur_file_system_;
}

Str FileSystem::ToNativeString(const Path &path) const {
#ifdef ION_PLATFORM_WINDOWS
    return path.make_preferred().string();
#else
    return path.native();
#endif
}

bool FileSystem::Exists(const Path &path) const {
    return std::filesystem::exists(path);
}

bool FileSystem::IsDirectory(const Path &path) const {
    return std::filesystem::is_directory(path);
}

bool FileSystem::IsAbsolute(const Path &path) const {
    return path.is_absolute();
}

bool FileSystem::IsHidden(const Path &path) const {
#ifdef ION_PLATFORM_WINDOWS
    return Exists(path) &&
        (GetFileAttributes(path.wstring().c_str()) & FILE_ATTRIBUTE_HIDDEN);
#else
    return Exists(path) && path.filename().string().starts_with('.');
#endif
}

UTime FileSystem::GetModTime(const Path &path) const {
    ASSERT(Exists(path));
    return UTime(std::filesystem::last_write_time(path));
}

void FileSystem::GetDirectoryContents(const Path &path,
                                      StrVec &subdirs, StrVec &files,
                                      bool include_hidden) const {
    ASSERT(IsDirectory(path));

    subdirs.clear();
    files.clear();

    for (const auto &entry: std::filesystem::directory_iterator(path)) {
        if (! include_hidden && IsHidden(entry.path()))
            continue;

        const Str name = entry.path().filename().string();

        if (entry.is_regular_file())
            files.push_back(name);

        // Filter out "." and ".." directories.
        else if (entry.is_directory() && (name != "." && name != ".."))
            subdirs.push_back(name);
    }

    std::sort(subdirs.begin(), subdirs.end());
    std::sort(files.begin(),   files.end());
}

// LCOV_EXCL_START [difficult or dangerous to test]
void FileSystem::Remove(const Path &path) const {
    ASSERT(Exists(path));
    std::filesystem::remove(path);
}

bool FileSystem::CreateDirectories(const Path &path) const {
    std::error_code ec;
    bool ret = std::filesystem::create_directories(path, ec);

    // Windows has a bug in create_directories(const Path &path); it returns
    // false with an error code of 0 if the directory already exists.
    if (! ret && ec.value() == 0)
        ret = true;

    // Must be an actual failure or the result is a directory.
    ASSERT(! ret || IsDirectory(path));

    return ret;
}

void FileSystem::MakeCurrent(const Path &path) const {
    std::filesystem::current_path(path);
}

FileSystem::Path FileSystem::GetCurrent() const {
    return std::filesystem::current_path();
}

FileSystem::Path FileSystem::GetHomeDirPath() const {
#ifdef ION_PLATFORM_WINDOWS
    const Str kVarName = "USERPROFILE";
#else
    const Str kVarName = "HOME";
#endif
    const auto dir = GetEnvVar_(kVarName);
    ASSERTM(Exists(dir), dir);
    return dir;
}

FileSystem::Path FileSystem::GetSettingsDirPath(const Str &app_name) const {
#ifdef ION_PLATFORM_WINDOWS
    auto path = Path(GetEnvVar_("APPDATA"));
#else
    auto path = Path(GetEnvVar_("HOME")) / ".config";
#endif
    return path / app_name;
}

FileSystem::Path FileSystem::GetTempFilePath() const {
    return std::filesystem::temp_directory_path();
}

Str FileSystem::GetSeparator() const {
#ifdef ION_PLATFORM_WINDOWS
    return Util::FromWString(std::wstring(1, Path::preferred_separator));
#else
    return Str(1, Path::preferred_separator);
#endif
}

Str FileSystem::GetEnvVar_(const Str &name) {
    return ion::port::GetEnvironmentVariableValue(name);
}
// LCOV_EXCL_STOP
