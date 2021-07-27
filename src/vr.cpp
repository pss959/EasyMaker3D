#include "vr.h"

#include <iostream>
#include <string>

#include <GL/glx.h>
#include <X11/Xlib.h>
#include <string.h>

// before including openxr_platform.h we have to define which platform specific parts we want enabled
#define XR_USE_PLATFORM_XLIB
#define XR_USE_GRAPHICS_API_OPENGL
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

namespace vr {

static XrInstance s_instance;

static bool Check(XrResult result, const char *what) {
    if (! XR_SUCCEEDED(result)) {
	char msg[XR_MAX_RESULT_STRING_SIZE];
	xrResultToString(nullptr, result, msg);
        std::cerr << "*** " << what << " failed with result " << result
                  << ": " << msg << "\n";
        return false;
    }
    return true;
}

bool Init() {
    const char *extensions[] = { XR_KHR_OPENGL_ENABLE_EXTENSION_NAME };
    uint32_t extension_count = sizeof(extensions) / sizeof(extensions[0]);

    XrInstanceCreateInfo instanceCreateInfo = {
        .type = XR_TYPE_INSTANCE_CREATE_INFO,
        .next = NULL,
        .createFlags = 0,
        .applicationInfo =  {
            .applicationVersion = 1,
            .engineVersion = 0,
            .apiVersion = XR_CURRENT_API_VERSION,
        },
        .enabledApiLayerCount = 0,
        .enabledExtensionCount = extension_count,
        .enabledExtensionNames = (const char * const *) extensions,
    };

    strncpy(instanceCreateInfo.applicationInfo.applicationName,
            "VR Test", XR_MAX_APPLICATION_NAME_SIZE);
    strncpy(instanceCreateInfo.applicationInfo.engineName,
            "VR Engine", XR_MAX_APPLICATION_NAME_SIZE);

    XrResult result = xrCreateInstance(&instanceCreateInfo, &s_instance);
    if (! Check(result, "xrCreateInstance"))
        return false;

    PFN_xrGetOpenGLGraphicsRequirementsKHR
        pfnGetOpenGLGraphicsRequirementsKHR = nullptr;
    result = xrGetInstanceProcAddr(
        s_instance, "xrGetOpenGLGraphicsRequirementsKHR",
        (PFN_xrVoidFunction *) &pfnGetOpenGLGraphicsRequirementsKHR);
    if (! Check(result, "Get OpenGL graphics requirements function"))
        return false;

    XrFormFactor form_factor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrSystemGetInfo system_get_info = {
        .type = XR_TYPE_SYSTEM_GET_INFO,
        .next = nullptr,
        .formFactor = form_factor,
    };
    XrSystemId system_id = XR_NULL_SYSTEM_ID;
    result = xrGetSystem(s_instance, &system_get_info, &system_id);
    if (! Check(result, "xrGetSystem"))
        return false;
    std::cout << "XXXX Got XrSystem with id " << system_id << "\n";

    // OpenXR requires checking graphics requirements before creating a session.
    XrGraphicsRequirementsOpenGLKHR opengl_reqs = {
        .type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR,
        .next = nullptr,
    };
    result = pfnGetOpenGLGraphicsRequirementsKHR(s_instance, system_id,
                                                 &opengl_reqs);
    if (! Check(result, "Get OpenGL graphics requirements!"))
        return false;

    XrGraphicsBindingOpenGLXlibKHR graphics_binding_gl =
	(XrGraphicsBindingOpenGLXlibKHR){
        .type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR,
    };
    XrSessionCreateInfo session_create_info = {
        .type = XR_TYPE_SESSION_CREATE_INFO,
        .next = &graphics_binding_gl,
        .systemId = system_id,
    };
    XrSession session = XR_NULL_HANDLE;
    result = xrCreateSession(s_instance, &session_create_info, &session);
    if (! Check(result, "xrCreateSession"))
        return false;

    return true;
}

}  // namespace gfx
