#include "Util/URL.h"

#if defined(ION_PLATFORM_LINUX)
#include <cstdlib>

#elif defined(ION_PLATFORM_MAC)

#elif defined(ION_PLATFORM_WINDOWS)
#include <windows.h>
#include <shellapi.h>

#endif

#include "Util/String.h"

namespace Util {

void OpenURL(const std::string &url) {

#if defined(ION_PLATFORM_LINUX)
    const std::string cmd = "xdg-open " + url;
    std::system(cmd.c_str());

#elif defined(ION_PLATFORM_MAC)
    const std::string cmd = "open " + url;
    std::system(cmd.c_str());

#elif defined(ION_PLATFORM_WINDOWS)
    ShellExecute(nullptr, nullptr, ToWString(url).c_str(),
                 nullptr, nullptr, SW_SHOWNORMAL);

#else
    ASSERTM(false, "Unknown platform for OpenURL");
#endif
}

}  // namespace Util
