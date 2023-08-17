#include "Util/URL.h"

#ifdef ION_PLATFORM_LINUX
#include <cstdlib>

#elifdef ION_PLATFORM_MAC

#elifdef ION_PLATFORM_WINDOWS
#include <windows.h>
#include <shellapi.h>

#endif

#include "Util/Assert.h"
#include "Util/String.h"

namespace Util {

// LCOV_EXCL_START
void OpenURL(const Str &url) {

#ifdef ION_PLATFORM_LINUX
    const Str cmd = "xdg-open " + url;
    if (std::system(cmd.c_str()) < 0) {
        ASSERTM(false, "std::system() failed");
    }

#elifdef ION_PLATFORM_MAC
    const Str cmd = "open " + url;
    std::system(cmd.c_str());

#elifdef ION_PLATFORM_WINDOWS
    ShellExecute(nullptr, nullptr, ToWString(url).c_str(),
                 nullptr, nullptr, SW_SHOWNORMAL);

#else
    ASSERTM(false, "Unknown platform for OpenURL");
#endif
}
// LCOV_EXCL_STOP

}  // namespace Util
