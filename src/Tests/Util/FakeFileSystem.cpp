//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/Util/FakeFileSystem.h"

#include <ion/base/stringutils.h>

#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/String.h"

void FakeFileSystem::AddFile(const Str &path_str, bool is_hidden) const {
    file_map_[path_str] = PathInfo_(false, is_hidden);
}

void FakeFileSystem::AddDir(const Str &path_str, bool is_hidden) const {
    file_map_[path_str] = PathInfo_(true, is_hidden);
}

void FakeFileSystem::SetModTime(const Str &path_str, const UTime &time) const {
    auto it = file_map_.find(path_str);
    ASSERT(it != file_map_.end());
    it->second.mod_time = time;
}

Str FakeFileSystem::ToNativeString(const Path &path) const {
    return path.string();
}

bool FakeFileSystem::Exists(const Path &path) const {
    return file_map_.contains(path.string());
}

bool FakeFileSystem::IsDirectory(const Path &path) const {
    const auto it = file_map_.find(path.string());
    return it != file_map_.end() && it->second.is_dir;
}

bool FakeFileSystem::IsAbsolute(const Path &path) const {
    return path.string().starts_with("/");
}

bool FakeFileSystem::IsHidden(const Path &path) const {
    const auto it = file_map_.find(path.string());
    return it != file_map_.end() && it->second.is_hidden;
}

UTime FakeFileSystem::GetModTime(const Path &path) const {
    auto it = file_map_.find(path.string());
    ASSERT(it != file_map_.end());
    return it->second.mod_time;
}

void FakeFileSystem::GetDirectoryContents(const Path &path,
                                          StrVec &subdirs, StrVec &files,
                                          bool include_hidden) const {
    ASSERT(IsDirectory(path));

    subdirs.clear();
    files.clear();

    // Look through the map for paths that start with the directory path.
    const Str pstr = path.string() + "/";
    for (const auto &entry: file_map_) {
        const Str &s = entry.first;
        if (! s.starts_with(pstr))
            continue;

        // These have to be direct children.
        const Str name = s.substr(pstr.size());
        if (name.contains("/"))
            continue;

        if (! include_hidden && entry.second.is_hidden)
            continue;

        if (entry.second.is_dir)
            subdirs.push_back(name);
        else
            files.push_back(name);
    }

    std::sort(subdirs.begin(), subdirs.end());
    std::sort(files.begin(),   files.end());
}

void FakeFileSystem::Remove(const Path &path) const {
    ASSERT(Exists(path));
    file_map_.erase(path.string());
}

bool FakeFileSystem::CreateDirectories(const Path &path) const {
    // Works only with absolute paths.
    ASSERT(IsAbsolute(path));

    // Make sure each component of the path is a known directory.
    Str cur_path;
    for (const auto &dir: ion::base::SplitString(path.string(), "/")) {
        cur_path += "/" + dir;
        if (! IsDirectory(cur_path))
            AddDir(cur_path);
    }
    return true;
}

void FakeFileSystem::MakeCurrent(const Path &path) const {
    current_ = path;
}

FakeFileSystem::Path FakeFileSystem::GetCurrent() const {
    return current_;
}

FakeFileSystem::Path FakeFileSystem::GetHomeDirPath() const {
    return "/home/user";
}

FakeFileSystem::Path FakeFileSystem::GetSettingsDirPath(
    const Str &app_name) const {
    return "/settings/" + app_name;
}

Str FakeFileSystem::GetSeparator() const {
    return "/";
}
