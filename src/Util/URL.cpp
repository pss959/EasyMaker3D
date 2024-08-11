//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Util/URL.h"

#ifdef ION_PLATFORM_LINUX
#include <cstdlib>

#elifdef ION_PLATFORM_MAC

#elifdef ION_PLATFORM_WINDOWS
#include <windows.h>
#include <shellapi.h>

#endif

#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/String.h"

namespace {

static std::function<void(const Str &)> s_open_url_func_;

}  // anonymous namespace

namespace Util {

void SetOpenURLFunc(const std::function<void(const Str &)> &func) {
    // This should be called only in unit tests.
    ASSERT(Util::app_type == Util::AppType::kUnitTest);
    s_open_url_func_ = func;
}

void OpenURL(const Str &url) {
    if (s_open_url_func_) {
        s_open_url_func_(url);
    }
    else {  // LCOV_EXCL_START [cannot test in isolation]

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
    } // LCOV_EXCL_STOP
}

}  // namespace Util
