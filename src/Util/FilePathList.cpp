#include "Util/FilePathList.h"

#include <iostream>

#include "Util/Assert.h"

void FilePathList::Init(const FilePath &initial_path) {
    ASSERT(initial_path.IsAbsolute());
    paths_.clear();
    paths_.push_back(initial_path);
    cur_index_ = 0;
}

bool FilePathList::CanGoInDirection(Direction dir) const {
    switch (dir) {
      case Direction::kUp:
        return IsValidDirectory(GetCurrent().GetParentDirectory());
      case Direction::kForward:
        return cur_index_ + 1 < paths_.size();
      case Direction::kBack:
        return cur_index_ > 0;
      case Direction::kHome:
        return GetCurrent() != FilePath::GetHomeDirPath();
    }
    ASSERT(false);  // LCOV_EXCL_LINE
    return false;   // LCOV_EXCL_LINE
}

const FilePath & FilePathList::GoInDirection(Direction dir) {
    ASSERT(CanGoInDirection(dir));
    switch (dir) {
      case Direction::kUp:
        AddPath(GetCurrent().GetParentDirectory());
        break;
      case Direction::kForward:
        ++cur_index_;
        break;
      case Direction::kBack:
        --cur_index_;
        break;
      case Direction::kHome:
        AddPath(FilePath::GetHomeDirPath());
        break;
      default:          // LCOV_EXCL_LINE
        ASSERT(false);  // LCOV_EXCL_LINE
    }
    return GetCurrent();
}

const FilePath & FilePathList::AddPath(const FilePath &path) {
    ASSERT(! paths_.empty());

    const FilePath full_path = MakeAbsolute_(path);

    if (full_path != paths_.back()) {
        // Remove anything after the current path. This gets rid of old history
        // when the user just went back to a previous path.
        paths_.erase(paths_.begin() + cur_index_ + 1, paths_.end());
        cur_index_ = paths_.size();
        paths_.push_back(full_path);
    }
    return GetCurrent();
}

// LCOV_EXCL_START
void FilePathList::GetContents(StrVec &subdirs, StrVec &files,
                               const Str &extension,
                               bool include_hidden) const {
    // If the current path is a directory, use it. Otherwise, use its parent.
    FilePath dir = GetCurrent();
    if (! dir.IsDirectory())
        dir = dir.GetParentDirectory();

    // Get sorted lists of directories and files in the current directory.
    dir.GetContents(subdirs, files, extension, include_hidden);
}

bool FilePathList::IsValidDirectory(const FilePath &path) const {
    return path.IsDirectory();
}

bool FilePathList::IsExistingFile(const FilePath &path) const {
    return path.Exists();
}

FilePath FilePathList::MakeAbsolute_(const FilePath &path) {
    if (path.IsAbsolute())
        return path;
    FilePath dir = GetCurrent();
    if (! dir.IsDirectory())
        dir = dir.GetParentDirectory();
    return FilePath::Join(dir, path);
}

void FilePathList::Dump() {
    for (size_t i = 0; i < paths_.size(); ++i)
        std::cout << "[" << i << "] " << paths_[i].ToString()
                  << (i == cur_index_ ? " [CURRENT]\n" : "\n");
}
// LCOV_EXCL_STOP
