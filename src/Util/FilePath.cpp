//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Util/FilePath.h"

#include <algorithm>

#include "Util/FileSystem.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

FilePath & FilePath::operator=(const char *path) {
    FSPath_::operator=(path);
    return *this;
}

FilePath & FilePath::operator=(const Str &path) {
    FSPath_::operator=(path);
    return *this;
}

Str FilePath::ToString() const {
    return generic_string();
}

Str FilePath::ToNativeString() const {
    return GetFS_().ToNativeString(*this);
}

bool FilePath::Exists() const {
    return GetFS_().Exists(*this);
}

bool FilePath::IsDirectory() const {
    return GetFS_().IsDirectory(*this);
}

bool FilePath::IsAbsolute() const {
    return GetFS_().IsAbsolute(*this);
}

bool FilePath::IsHidden() const {
    return GetFS_().IsHidden(*this);
}

FilePath FilePath::GetParentDirectory() const {
    return FromFSPath_(parent_path());
}

Str FilePath::GetFileName(bool remove_extension) const {
    return FromFSPath_(remove_extension ? stem() : filename()).ToString();
}

Str FilePath::GetExtension() const {
    return FromFSPath_(extension()).ToString();
}

void FilePath::AddExtension(const Str &extension) {
    ASSERT(extension.starts_with('.') && extension.size() > 1U);
    Str s = ToString();
    // If the extension is already present, do nothing.
    if (! s.ends_with(extension)) {
        // Remove any trailing dots and slashes
        while (s.ends_with('.') || s.ends_with('/'))
            s.resize(s.size() - 1);
        *this = FromFSPath_(s + extension);
    }
}

void FilePath::ReplaceExtension(const Str &extension) {
    replace_extension(extension);
}

FilePath FilePath::AppendRelative(const FilePath &base_path) const {
    if (IsAbsolute())
        return *this;

    // If the base_path exists and is not known to be a directory, remove the
    // last component.
    else if (GetFS_().Exists(base_path) &&
             ! GetFS_().IsDirectory(base_path))
        return FromFSPath_(base_path.parent_path() / *this);
    else
        return FromFSPath_(base_path / *this);
}

FilePath FilePath::MakeRelativeTo(const FilePath &base_path) const {
    return FromFSPath_(lexically_proximate(base_path));
}

FilePath FilePath::GetAbsolute() const {
    return FromFSPath_(IsAbsolute() ? lexically_normal() :
                       (GetFS_().GetCurrent() / *this));
}

UTime FilePath::GetModTime() const {
    ASSERT(Exists());
    return GetFS_().GetModTime(*this);
}

void FilePath::GetContents(StrVec &subdirs, StrVec &files, const Str &extension,
                           bool include_hidden) const {
    if (! IsDirectory()) {
        subdirs.clear();
        files.clear();
        return;
    }

    GetFS_().GetDirectoryContents(*this, subdirs, files,
                                           include_hidden);

    // Filter out by extension if requested.
    if (! extension.empty())
        std::erase_if(files, [&](const Str &f){
            return FilePath(f).GetExtension() != extension; });
}

void FilePath::Remove() const {
    KLOG('f', "Removing path \"" << ToString() << "\n");
    ASSERT(Exists());
    GetFS_().Remove(*this);
}

bool FilePath::CreateDirectories() const {
    KLOG('f', "Creating directories for path \"" << ToString() << "\"");
    return GetFS_().CreateDirectories(*this);
}

void FilePath::MakeCurrent() const {
    GetFS_().MakeCurrent(*this);
}

FilePath FilePath::GetCurrent() {
    return FromFSPath_(GetFS_().GetCurrent());
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

FilePath FilePath::GetResourcePath(const Str &type_name,
                                   const FilePath &sub_path) {
    FilePath path = GetResourceBasePath();
    path /= type_name;
    path /= sub_path;
    return path;
}

FilePath FilePath::GetFullResourcePath(const Str &subdir,
                                       const FilePath &path) {
    if (path.IsAbsolute())
        return path;
    else
        return GetResourcePath(subdir, path);
}

FilePath FilePath::GetHomeDirPath() {
    return FromFSPath_(GetFS_().GetHomeDirPath());
}

FilePath FilePath::GetSettingsDirPath(const Str &app_name) {
    return FromFSPath_(GetFS_().GetSettingsDirPath(app_name));
}

FilePath FilePath::GetTestDataPath() {
    return FilePath(TEST_DATA_DIR);
}

FilePath FilePath::GetTempFilePath() {
    return FromFSPath_(GetFS_().GetTempFilePath());
}

Str FilePath::GetSeparator() {
    return GetFS_().GetSeparator();
}

FileSystem & FilePath::GetFS_() {
    return *FileSystem::GetInstalled();
}

FilePath FilePath::FromFSPath_(const FSPath_ &fs_path) {
#ifdef ION_PLATFORM_WINDOWS
    return FilePath(fs_path.string());
#else
    return FilePath(fs_path);
#endif
}
