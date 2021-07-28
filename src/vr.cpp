#include "vr.h"

#include <iostream>
#include <string>
#include <vector>

#include <string.h>

#include <X11/Xlib.h>
#include <GL/glx.h>

#define XR_USE_PLATFORM_XLIB
#define XR_USE_GRAPHICS_API_OPENGL
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include "gfx.h"

// ----------------------------------------------------------------------------
// Helper_ class definition.
// ----------------------------------------------------------------------------

class VR::Helper_ {
  public:
    // Initialization.
    bool CreateInstance();
    bool GetSystem();
    bool InitViews();
    bool CreateSession(const GFX &gfx);

    // Query.
    int GetWidth()  const { return width_;  }
    int GetHeight() const { return height_; }

  private:
    XrInstance instance_   = nullptr;
    XrSystemId system_id_  = XR_NULL_SYSTEM_ID;
    uint32_t   view_count_ = 0;
    XrSession  session_    = XR_NULL_HANDLE;
    int        width_      = 0;
    int        height_     = 0;

    void PrintInstanceProperties_();
    bool Check_(XrResult result, const char *what);
};

// ----------------------------------------------------------------------------
// VR class.
// ----------------------------------------------------------------------------

VR::VR() : helper_(new Helper_()) {
}

VR::~VR() {
}

bool VR::Init() {
    if (! helper_->CreateInstance()    ||
        ! helper_->GetSystem()         ||
        ! helper_->InitViews())
        return false;
    return true;
}

int VR::GetWidth() {
    return helper_->GetWidth();
}

int VR::GetHeight() {
    return helper_->GetHeight();
}

bool VR::CreateSession(const GFX &gfx) {
    return helper_->CreateSession(gfx);
}

// ----------------------------------------------------------------------------
// Helper_ class functions.
// ----------------------------------------------------------------------------

bool VR::Helper_::CreateInstance() {
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

    XrResult result = xrCreateInstance(&instanceCreateInfo, &instance_);
    return Check_(result, "xrCreateInstance");
}

bool VR::Helper_::GetSystem() {
    PFN_xrGetOpenGLGraphicsRequirementsKHR
        pfnGetOpenGLGraphicsRequirementsKHR = nullptr;
    XrResult result = xrGetInstanceProcAddr(
        instance_, "xrGetOpenGLGraphicsRequirementsKHR",
        (PFN_xrVoidFunction *) &pfnGetOpenGLGraphicsRequirementsKHR);
    if (! Check_(result, "Get OpenGL graphics requirements function"))
        return false;

    PrintInstanceProperties_();

    XrFormFactor form_factor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrSystemGetInfo system_get_info = {
        .type = XR_TYPE_SYSTEM_GET_INFO,
        .next = nullptr,
        .formFactor = form_factor,
    };
    result = xrGetSystem(instance_, &system_get_info, &system_id_);
    if (! Check_(result, "xrGetSystem"))
        return false;
    std::cout << "XXXX Got XrSystem with id " << system_id_ << "\n";

    // OpenXR requires checking graphics requirements before creating a session.
    XrGraphicsRequirementsOpenGLKHR reqs = {
        .type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR,
        .next = nullptr,
    };
    result = pfnGetOpenGLGraphicsRequirementsKHR(instance_, system_id_, &reqs);
    return Check_(result, "Get OpenGL graphics requirements!");
}

bool VR::Helper_::InitViews() {
    XrViewConfigurationType view_type =
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    XrResult result = xrEnumerateViewConfigurationViews(
        instance_, system_id_, view_type, 0, &view_count_, nullptr);
    if (! Check_(result, "xrEnumerateViewConfigurationViews"))
        return false;
    std::cout << "XXXX Got " << view_count_ << " view(s)\n";

    std::vector<XrViewConfigurationView> views(view_count_);
    for (uint32_t i = 0; i < view_count_; i++) {
        views[i].type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
        views[i].next = nullptr;
    }

    result = xrEnumerateViewConfigurationViews(
        instance_, system_id_, view_type, view_count_, &view_count_, &views[0]);
    if (! Check_(result, "xrEnumerateViewConfigurationViews"))
        return false;

    width_  = views[0].recommendedImageRectWidth;
    height_ = views[0].recommendedImageRectHeight;
    std::cout << "XXXX Recommended width: "
              << width_ << ", height: " << height_ << "\n";
    return true;
}

bool VR::Helper_::CreateSession(const GFX &gfx) {
    std::cout << "XXXX OpenGL Version:  " << glGetString(GL_VERSION)  << "\n";
    std::cout << "XXXX OpenGL Renderer: " << glGetString(GL_RENDERER) << "\n";

    XrGraphicsBindingOpenGLXlibKHR binding = {
        .type        = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR,
        .xDisplay    = gfx.GetDisplay(),
        .glxDrawable = gfx.GetDrawable(),
        .glxContext  = gfx.GetContext(),
    };
    XrSessionCreateInfo info = {
        .type = XR_TYPE_SESSION_CREATE_INFO,
        .next = &binding,
        .systemId = system_id_,
    };
    XrResult result = xrCreateSession(instance_, &info, &session_);
    if (! Check_(result, "xrCreateSession"))
        return false;

    std::cout << "XXXX Created session with OpenGL.\n";
    return true;
}

void VR::Helper_::PrintInstanceProperties_() {
    XrInstanceProperties props = {
        .type = XR_TYPE_INSTANCE_PROPERTIES,
        .next = nullptr,
    };
    XrResult result = xrGetInstanceProperties(instance_, &props);
    if (! Check_(result, "xrGetInstanceProperties"))
        return;

    std::cerr << "XXXX Runtime Name:    " << props.runtimeName << "\n";
    std::cerr << "XXXX Runtime Version: "
              << XR_VERSION_MAJOR(props.runtimeVersion) << "."
              << XR_VERSION_MINOR(props.runtimeVersion) << "."
              << XR_VERSION_PATCH(props.runtimeVersion) << "\n";
}

bool VR::Helper_::Check_(XrResult result, const char *what) {
    if (XR_SUCCEEDED(result))
        return true;

    std::cerr << "*** " << what << " failed with result " << result;
    if (instance_) {
        char msg[XR_MAX_RESULT_STRING_SIZE] = "";
        xrResultToString(instance_, result, msg);
        std::cerr << ": " << msg << "\n";
    }
    else {
        std::cerr << "\n";
    }
    return false;
}

