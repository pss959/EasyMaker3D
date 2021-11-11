#include "VR/VRBase.h"

#include <iostream>
#include <sstream>

#include "Util/Enum.h"

// Set this to 1 to trace all OpenXR calls.
#define TRACE_XR 0

void VRBase::CheckXr_(XrResult res, const char *cmd,
                      const char *file, int line) {
#if TRACE_XR
    std::cout << "==== <" << cmd << "> result: " << Util::EnumName(res) << "\n";
#endif
    if (XR_FAILED(res)) {
        char buf[XR_MAX_RESULT_STRING_SIZE];
        std::string res_str;
        if (XR_SUCCEEDED(xrResultToString(instance_, res, buf)))
            res_str = buf;
        else
            res_str = "<Unknown result>";
        std::ostringstream out;
        out << "***OpenXR failure: result=" << res << " (" << res_str
            << ") " << cmd << " " << file << ":" << line;
        Throw_(out.str());
    }
}

void VRBase::Assert_(bool exp, const char *expstr, const char *file, int line) {
    if (! exp) {
        std::ostringstream out;
        out << "***Assertion failure: <" << expstr
            << "> at " << file << ":" << line;
        Throw_(out.str());
    }
}

void VRBase::Throw_(const std::string &msg) {
    // std::cerr << "**************** " << msg << "\n";
    throw VRException_(msg);
}

void VRBase::ReportException_(const VRException_ &ex) {
    std::cerr << ex.what() << "\n";
}

void VRBase::ReportDisaster_(const char *msg) {
    std::cerr << "*** " << msg << ": Expect disaster\n";
}
