#include "Util/FilePath.h"

#ifdef ION_PLATFORM_WINDOWS
#include <fileapi.h>
#endif

#include <algorithm>

#include <ion/port/environment.h>

#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

/// Platform-dependent path construction.
static inline FilePath FromPath_(const std::filesystem::path &path) {
#ifdef ION_PLATFORM_WINDOWS
    return FilePath(path.string());
#else
    return FilePath(path);
#endif
}

/// Access to environment variables.
static std::string GetEnvVar_(const std::string &name) {
    // No regular access to environment in unit tests.
    return Util::app_type == Util::AppType::kInteractive ?
        ion::port::GetEnvironmentVariableValue(name) : "/";
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// FilePath functions.
// ----------------------------------------------------------------------------

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

// LCOV_EXCL_START
std::string FilePath::ToNativeString() const {
#ifdef ION_PLATFORM_WINDOWS
    return FilePath(*this).make_preferred().string();
#else
    return native();
#endif
}
// LCOV_EXCL_STOP

bool FilePath::Exists() const {
    return std::filesystem::exists(*this);
}

bool FilePath::IsDirectory() const {
    return std::filesystem::is_directory(*this);
}

bool FilePath::IsAbsolute() const {
    return is_absolute();
}

// LCOV_EXCL_START
bool FilePath::IsHidden() const {
#ifdef ION_PLATFORM_WINDOWS
    return Exists() && (GetFileAttributes(Util::ToWString(ToString()).c_str()) &
                        FILE_ATTRIBUTE_HIDDEN);
#else
    return Exists() && GetFileName()[0] == '.';
#endif
}
// LCOV_EXCL_STOP

FilePath FilePath::GetParentDirectory() const {
    return FromPath_(parent_path());
}

std::string FilePath::GetFileName(bool remove_extension) const {
    return FromPath_(remove_extension ? stem() : filename()).ToString();
}

std::string FilePath::GetExtension() const {
    return FromPath_(extension()).ToString();
}

void FilePath::AddExtension(const std::string &extension) {
    replace_extension(extension);
}

FilePath FilePath::AppendRelative(const FilePath &base_path) const {
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

FilePath FilePath::MakeRelativeTo(const FilePath &base_path) const {
    return FromPath_(lexically_proximate(base_path));
}

FilePath FilePath::GetAbsolute() const {
    return FromPath_(IsAbsolute() ? lexically_normal() :
                     (std::filesystem::current_path() / *this));
}

UTime FilePath::GetModTime() const {
    ASSERT(Exists());
    return UTime(std::filesystem::last_write_time(*this));
}

// LCOV_EXCL_START
void FilePath::GetContents(std::vector<std::string> &subdirs,
                           std::vector<std::string> &files,
                           const std::string &extension,
                           bool include_hidden) const {
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
        else if (entry.is_regular_file()) {
            if (extension.empty() || entry.path().extension() == extension)
                files.push_back(name);
        }
    }

    std::sort(subdirs.begin(), subdirs.end());
    std::sort(files.begin(),   files.end());
}

void FilePath::Remove() const {
    KLOG('f', "Removing path \"" << ToString() << "\n");
    ASSERT(Exists());
    std::filesystem::remove(*this);
}

void FilePath::CopyTo(const FilePath &to_path) const {
    std::filesystem::copy(*this, to_path);
}

bool FilePath::CreateDirectories() const {
    KLOG('f', "Creating directories for path \"" << ToString() << "\"");
    std::error_code ec;
    bool ret = std::filesystem::create_directories(*this, ec);

    // Windows has a bug in create_directories(); it returns false with an
    // error code of 0 if the directory already exists.
    if (! ret && ec.value() == 0)
        ret = true;

    // Must be an actual failure or the result is a directory.
    ASSERT(! ret || IsDirectory());

    return ret;
}

void FilePath::MakeCurrent() const {
    std::filesystem::current_path(*this);
}
// LCOV_EXCL_STOP

FilePath FilePath::GetCurrent() {
    return FromPath_(std::filesystem::current_path());
}

FilePath FilePath::Join(const FilePath &p0, const FilePath &p1) {
    ASSERT(! p1.IsAbsolute());
    FilePath result = p0;
    result /= p1;
    return result;
}

// LCOV_EXCL_START
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
#ifdef ION_PLATFORM_WINDOWS
    const std::string kVarName = "USERPROFILE";
#else
    const std::string kVarName = "HOME";
#endif
    const FilePath dir = GetEnvVar_(kVarName);
    ASSERTM(dir.Exists(), dir.ToString());
    return dir;
}

FilePath FilePath::GetSettingsDirPath(const std::string &app_name) {
#ifdef ION_PLATFORM_WINDOWS
    FilePath path = GetEnvVar_("APPDATA");
#else
    FilePath path = Join(FilePath(GetEnvVar_("HOME")), FilePath(".config"));
#endif
    return Join(path, FilePath(app_name));
}

FilePath FilePath::GetTestDataPath() {
    return FilePath(TEST_DATA_DIR);
}

FilePath FilePath::GetTempFilePath() {
    return FromPath_(std::filesystem::temp_directory_path());
}

std::string FilePath::GetSeparator() {
#ifdef ION_PLATFORM_WINDOWS
    return Util::FromWString(std::wstring(1, preferred_separator));
#else
    return std::string(1, preferred_separator);
#endif
}
// LCOV_EXCL_STOP
