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
            "Example Application", XR_MAX_APPLICATION_NAME_SIZE);
    strncpy(instanceCreateInfo.applicationInfo.engineName,
            "Example Engine", XR_MAX_APPLICATION_NAME_SIZE);

    XrInstance instance;
    XrResult result = xrCreateInstance(&instanceCreateInfo, &instance);
    return XR_SUCCEEDED(result);
}

}  // namespace gfx
