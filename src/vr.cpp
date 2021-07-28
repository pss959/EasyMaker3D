#include "vr.h"

#include <algorithm>
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
    int GetWidth() const {
        return views_[0].recommendedImageRectWidth;
    }
    int GetHeight() const {
        return views_[0].recommendedImageRectHeight;
    }

  private:
    typedef std::vector<std::vector<XrSwapchainImageOpenGLKHR>> SCImages_;

    XrInstance instance_   = nullptr;
    XrSystemId system_id_  = XR_NULL_SYSTEM_ID;
    XrSession  session_    = XR_NULL_HANDLE;

    std::vector<XrViewConfigurationView> views_;
    SCImages_                            color_images_;
    SCImages_                            depth_images_;

    void PrintInstanceProperties_();
    bool SetUpRendering_();
    bool SetUpReferenceSpace_();
    bool SetUpSwapchains_(int64_t format, XrSwapchainUsageFlags usage_flags,
                          SCImages_ &sc_images);
    int64_t GetSwapchainFormat_(int64_t preferred);
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
    uint32_t count = 0;
    XrResult result = xrEnumerateViewConfigurationViews(
        instance_, system_id_, view_type, 0, &count, nullptr);
    if (! Check_(result, "xrEnumerateViewConfigurationViews"))
        return false;

    views_.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        views_[i].type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
        views_[i].next = nullptr;
    }

    result = xrEnumerateViewConfigurationViews(
        instance_, system_id_, view_type, count, &count, &views_[0]);
    if (! Check_(result, "xrEnumerateViewConfigurationViews"))
        return false;

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

    return SetUpRendering_();
}

bool VR::Helper_::SetUpRendering_() {
    return SetUpReferenceSpace_() &&
        SetUpSwapchains_(GetSwapchainFormat_(GL_SRGB8_ALPHA8_EXT),
                         (XR_SWAPCHAIN_USAGE_SAMPLED_BIT |
                          XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT),
                         color_images_) &&
        SetUpSwapchains_(GetSwapchainFormat_(GL_DEPTH_COMPONENT16),
                         XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         depth_images_);
}

bool VR::Helper_::SetUpReferenceSpace_() {
    XrPosef identity_pose = {
        .orientation = {.x = 0, .y = 0, .z = 0, .w = 1.0},
        .position    = {.x = 0, .y = 0, .z = 0}
    };
    XrReferenceSpaceCreateInfo ps_info = {
        .type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO,
        .next = nullptr,
        .referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL,  // Seated.
        .poseInReferenceSpace = identity_pose
    };
    XrSpace play_space = XR_NULL_HANDLE;
    XrResult result = xrCreateReferenceSpace(session_, &ps_info, &play_space);
    return Check_(result, "xrCreateReferenceSpace");
}

bool VR::Helper_::SetUpSwapchains_(int64_t format,
                                   XrSwapchainUsageFlags usage_flags,
                                   SCImages_ &sc_images) {
    uint32_t sc_count;
    XrResult result = xrEnumerateSwapchainFormats(session_, 0,
                                                  &sc_count, nullptr);
    if (! Check_(result, "xrEnumerateSwapchainFormats"))
        return false;

    int64_t sc_formats[sc_count];
    result = xrEnumerateSwapchainFormats(session_, sc_count,
                                         &sc_count, sc_formats);
    if (! Check_(result, "xrEnumerateSwapchainFormats"))
        return false;

    size_t view_count = views_.size();
    std::vector<XrSwapchain>               swapchains(view_count);
    std::vector<uint32_t>                  sc_lengths(view_count);
    sc_images.resize(view_count);

    for (uint32_t i = 0; i < view_count; i++) {
        XrSwapchainCreateInfo sc_info = {
            .type        = XR_TYPE_SWAPCHAIN_CREATE_INFO,
            .next        = nullptr,
            .createFlags = 0,
            .usageFlags  = usage_flags,
            .format      = format,
            .sampleCount = views_[i].recommendedSwapchainSampleCount,
            .width       = views_[i].recommendedImageRectWidth,
            .height      = views_[i].recommendedImageRectHeight,
            .faceCount   = 1,
            .arraySize   = 1,
            .mipCount    = 1,
        };
        result = xrCreateSwapchain(session_, &sc_info, &swapchains[i]);
        if (! Check_(result, "xrCreateSwapchain"))
            return false;

        // The runtime controls how many textures we have to be able to render
        // to (e.g. "triple buffering")
        result = xrEnumerateSwapchainImages(swapchains[i], 0,
                                            &sc_lengths[i], nullptr);
        if (! Check_(result, "xrEnumerateSwapchainImages"))
            return false;

        sc_images[i].resize(sc_lengths[i]);
        for (uint32_t j = 0; j < sc_lengths[i]; j++) {
            sc_images[i][j].type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
            sc_images[i][j].next = nullptr;
        }
        result = xrEnumerateSwapchainImages(
            swapchains[i], sc_lengths[i], &sc_lengths[i],
            (XrSwapchainImageBaseHeader *) &sc_images[i][0]);
        if (! Check_(result, "xrEnumerateSwapchainImages"))
            return false;
    }
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

// Returns the preferred swapchain format if it is supported, otherwise returns
// the first supported format. Returns -1 on error.
int64_t VR::Helper_::GetSwapchainFormat_(int64_t preferred_format) {
    uint32_t count;
    XrResult result = xrEnumerateSwapchainFormats(session_, 0, &count, nullptr);
    if (! Check_(result, "xrEnumerateSwapchainFormats") || count <= 0)
        return -1;

    std::vector<int64_t> formats(count);
    result = xrEnumerateSwapchainFormats(session_, count, &count, &formats[0]);
    if (! Check_(result, "xrEnumerateSwapchainFormats"))
        return -1;

    if (std::find(formats.begin(), formats.end(),
                  preferred_format) != formats.end())
        return preferred_format;
    else
        return formats[0];
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
