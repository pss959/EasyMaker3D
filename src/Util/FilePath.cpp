#include "Util/FilePath.h"

#include <algorithm>

#include "Util/FileSystem.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

std::shared_ptr<FileSystem> FilePath::real_file_system_(new FileSystem);
std::shared_ptr<FileSystem> FilePath::cur_file_system_ = real_file_system_;

void FilePath::InstallFileSystem(const std::shared_ptr<FileSystem> &fs) {
    cur_file_system_ = fs ? fs : real_file_system_;
}

const std::shared_ptr<FileSystem> FilePath::GetInstalledFileSystem() {
    return cur_file_system_;
}

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
    return cur_file_system_->ToNativeString(*this);
}

bool FilePath::Exists() const {
    return cur_file_system_->Exists(*this);
}

bool FilePath::IsDirectory() const {
    return cur_file_system_->IsDirectory(*this);
}

bool FilePath::IsAbsolute() const {
    return cur_file_system_->IsAbsolute(*this);
}

bool FilePath::IsHidden() const {
    return cur_file_system_->IsHidden(*this);
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
    replace_extension(extension);
}

FilePath FilePath::AppendRelative(const FilePath &base_path) const {
    if (IsAbsolute())
        return *this;

    // If the base_path exists and is not known to be a directory, remove the
    // last component.
    else if (cur_file_system_->Exists(base_path) &&
             ! cur_file_system_->IsDirectory(base_path))
        return FromFSPath_(base_path.parent_path() / *this);
    else
        return FromFSPath_(base_path / *this);
}

FilePath FilePath::MakeRelativeTo(const FilePath &base_path) const {
    return FromFSPath_(lexically_proximate(base_path));
}

FilePath FilePath::GetAbsolute() const {
    return FromFSPath_(IsAbsolute() ? lexically_normal() :
                       (cur_file_system_->GetCurrent() / *this));
}

UTime FilePath::GetModTime() const {
    ASSERT(Exists());
    return cur_file_system_->GetModTime(*this);
}

void FilePath::GetContents(StrVec &subdirs, StrVec &files, const Str &extension,
                           bool include_hidden) const {
    if (! IsDirectory()) {
        subdirs.clear();
        files.clear();
        return;
    }

    cur_file_system_->GetDirectoryContents(*this, subdirs, files,
                                           include_hidden);

    // Filter out by extension if requested.
    if (! extension.empty())
        std::erase_if(files, [&](const Str &f){
            return FilePath(f).GetExtension() != extension; });
}

void FilePath::Remove() const {
    KLOG('f', "Removing path \"" << ToString() << "\n");
    ASSERT(Exists());
    cur_file_system_->Remove(*this);
}

bool FilePath::CreateDirectories() const {
    KLOG('f', "Creating directories for path \"" << ToString() << "\"");
    return cur_file_system_->CreateDirectories(*this);
}

void FilePath::MakeCurrent() const {
    cur_file_system_->MakeCurrent(*this);
}

FilePath FilePath::GetCurrent() {
    return FromFSPath_(cur_file_system_->GetCurrent());
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
    return FromFSPath_(cur_file_system_->GetHomeDirPath());
}

FilePath FilePath::GetSettingsDirPath(const Str &app_name) {
    return FromFSPath_(cur_file_system_->GetSettingsDirPath(app_name));
}

FilePath FilePath::GetTestDataPath() {
    return FilePath(TEST_DATA_DIR);
}

FilePath FilePath::GetTempFilePath() {
    return FromFSPath_(cur_file_system_->GetTempFilePath());
}

Str FilePath::GetSeparator() {
    return cur_file_system_->GetSeparator();
}

FilePath FilePath::FromFSPath_(const FSPath_ &fs_path) {
#ifdef ION_PLATFORM_WINDOWS
    return FilePath(fs_path.string());
#else
    return FilePath(fs_path);
#endif
}
