#include "vr.h"

#include <algorithm>
#include <iostream>
#include <sstream>
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
// Handy macros.
// ----------------------------------------------------------------------------

// Calls the given function and throws a VRException if the result indicates
// failure.
#define CHECK_XR_(cmd) CheckXr_(cmd, #cmd, __FILE__, __LINE__)

// Shorthand for OpenXR-required casts.
#define CAST_(to_type, var) reinterpret_cast<to_type>(var)

// ----------------------------------------------------------------------------
// Helper_ class definition.
// ----------------------------------------------------------------------------

class VR::Helper_ {
  public:
    ~Helper_();

    // Initialization.
    void CreateInstance();
    void GetSystem();
    void InitViews();
    void CreateSession(const GFX &gfx);
    bool PollEvents();

    // Query.
    int GetWidth() const {
        return views_[0].recommendedImageRectWidth;
    }
    int GetHeight() const {
        return views_[0].recommendedImageRectHeight;
    }

    // Rendering.
    void Draw(const GFX &gfx);

  private:
    typedef std::vector<XrViewConfigurationView>                Views_;
    typedef std::vector<std::vector<XrSwapchainImageOpenGLKHR>> SCImages_;

    XrInstance     instance_      = nullptr;
    XrSystemId     system_id_     = XR_NULL_SYSTEM_ID;
    XrSession      session_       = XR_NULL_HANDLE;
    XrSessionState session_state_ = XR_SESSION_STATE_UNKNOWN;
    const XrViewConfigurationType view_type_ =
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    Views_     views_;
    SCImages_  color_images_;
    SCImages_  depth_images_;

    // Main set-up functions.
    void SetUpRendering_(const GFX &gfx);
    void SetUpReferenceSpace_();
    void SetUpSwapchains_(int64_t format, XrSwapchainUsageFlags usage_flags,
                          SCImages_ &sc_images);
    void SetUpGFX_(const GFX &gfx);

    // Helpers.
    void    PrintInstanceProperties_();
    int64_t GetSwapchainFormat_(int64_t preferred);
    bool    GetNextEvent_(XrEventDataBuffer &event);
    bool    ProcessSessionStateChange_(
        const XrEventDataSessionStateChanged &event);
    bool    RenderLayer_(
        XrTime predicted_display_time,
        const std::vector<XrCompositionLayerProjectionView> &views,
        XrCompositionLayerProjection &layer);

    inline void Disaster_(const char *msg) {
        std::cerr << "*** " << msg << ": Expect disaster\n";
    }

    inline void CheckXr_(XrResult res, const char *cmd,
                         const char *file, int line) {
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
            throw VR::VRException(out.str());
        }
    }
};

// ----------------------------------------------------------------------------
// VR class.
// ----------------------------------------------------------------------------

VR::VR() : helper_(new Helper_()) {
}

VR::~VR() {
}

void VR::Init() {
    helper_->CreateInstance();
    helper_->GetSystem();
    helper_->InitViews();
}

int VR::GetWidth() {
    return helper_->GetWidth();
}

int VR::GetHeight() {
    return helper_->GetHeight();
}

void VR::CreateSession(const GFX &gfx) {
    helper_->CreateSession(gfx);
}

bool VR::PollEvents() {
    return helper_->PollEvents();
}

void VR::Draw(const GFX &gfx) {
    helper_->Draw(gfx);
}

// ----------------------------------------------------------------------------
// Helper_ class functions.
// ----------------------------------------------------------------------------

VR::Helper_::~Helper_() {
    depth_images_.clear();
    color_images_.clear();
    views_.clear();
    if (session_ != XR_NULL_HANDLE)
        xrDestroySession(session_);
    if (instance_ != XR_NULL_HANDLE)
        xrDestroyInstance(instance_);
}

void VR::Helper_::CreateInstance() {
    const char * extension = XR_KHR_OPENGL_ENABLE_EXTENSION_NAME;

    XrInstanceCreateInfo create_info{ XR_TYPE_INSTANCE_CREATE_INFO };
    create_info.enabledExtensionCount = 1;
    create_info.enabledExtensionNames = &extension;
    strcpy(create_info.applicationInfo.applicationName, "VR Test");
    create_info.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
    CHECK_XR_(xrCreateInstance(&create_info, &instance_));
}

void VR::Helper_::GetSystem() {
    PFN_xrGetOpenGLGraphicsRequirementsKHR
        pfnGetOpenGLGraphicsRequirementsKHR = nullptr;
    CHECK_XR_(xrGetInstanceProcAddr(
                  instance_, "xrGetOpenGLGraphicsRequirementsKHR",
                  (PFN_xrVoidFunction *) &pfnGetOpenGLGraphicsRequirementsKHR));
    PrintInstanceProperties_();

    XrFormFactor form_factor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrSystemGetInfo system_get_info{ XR_TYPE_SYSTEM_GET_INFO };
    system_get_info.formFactor = form_factor;
    CHECK_XR_(xrGetSystem(instance_, &system_get_info, &system_id_));
    std::cout << "XXXX Got XrSystem with id " << system_id_ << "\n";

    // OpenXR requires checking graphics requirements before creating a session.
    XrGraphicsRequirementsOpenGLKHR reqs;
    reqs.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR;
    CHECK_XR_(pfnGetOpenGLGraphicsRequirementsKHR(
                  instance_, system_id_, &reqs));
}

void VR::Helper_::InitViews() {
    uint32_t count = 0;
    CHECK_XR_(xrEnumerateViewConfigurationViews(
                  instance_, system_id_, view_type_, 0, &count, nullptr));
    std::cout << "XXXX View count = " << count << "\n";

    views_.resize(count,
                  XrViewConfigurationView{XR_TYPE_VIEW_CONFIGURATION_VIEW});

    CHECK_XR_(xrEnumerateViewConfigurationViews(
                  instance_, system_id_, view_type_, count,
                  &count, &views_[0]));
}

void VR::Helper_::CreateSession(const GFX &gfx) {
    std::cout << "XXXX OpenGL Version:  " << glGetString(GL_VERSION)  << "\n";
    std::cout << "XXXX OpenGL Renderer: " << glGetString(GL_RENDERER) << "\n";

    XrGraphicsBindingOpenGLXlibKHR binding;
    binding.type        = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR;
    binding.xDisplay    = gfx.GetDisplay();
    binding.glxDrawable = gfx.GetDrawable();
    binding.glxContext  = gfx.GetContext();

    XrSessionCreateInfo info{ XR_TYPE_SESSION_CREATE_INFO };
    info.systemId = system_id_;
    info.next     = &binding;
    CHECK_XR_(xrCreateSession(instance_, &info, &session_));

    std::cout << "XXXX Created session with OpenGL.\n";

    return SetUpRendering_(gfx);
}

bool VR::Helper_::PollEvents() {
    // Process all pending messages.
    bool keep_going = true;
    XrEventDataBuffer event;
    while (keep_going && GetNextEvent_(event)) {
        switch (event.type) {
          case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
            Disaster_("OpenXR instance loss pending");
            break;
          case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
            keep_going = ProcessSessionStateChange_(
                CAST_(const XrEventDataSessionStateChanged &, event));
            break;

          case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
          case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
          default:
            std::cout << "XXXX Ignoring event type " << event.type << "\n";
            break;
        }
    }
    return keep_going;
}

void VR::Helper_::Draw(const GFX &gfx) {
    XrFrameWaitInfo wait_info{XR_TYPE_FRAME_WAIT_INFO};
    XrFrameState    frame_state{XR_TYPE_FRAME_STATE};
    CHECK_XR_(xrWaitFrame(session_, &wait_info, &frame_state));

    XrFrameBeginInfo frame_begin_info{ XR_TYPE_FRAME_BEGIN_INFO };
    CHECK_XR_(xrBeginFrame(session_, &frame_begin_info));

    std::vector<XrCompositionLayerProjection>     layers;
    std::vector<XrCompositionLayerBaseHeader *>   layer_ptrs;
    std::vector<XrCompositionLayerProjectionView> layer_views;
    if (frame_state.shouldRender == XR_TRUE) {
        XrCompositionLayerProjection layer;
        layer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
        if (RenderLayer_(frame_state.predictedDisplayTime, layer_views, layer)) {
            layers.push_back(layer);
            layer_ptrs.push_back(
                (XrCompositionLayerBaseHeader *) &layers.back());
        }
    }

    XrFrameEndInfo frame_end_info{ XR_TYPE_FRAME_END_INFO };
    frame_end_info.displayTime          = frame_state.predictedDisplayTime;
    frame_end_info.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    frame_end_info.layerCount           = (uint32_t) layer_ptrs.size();
    frame_end_info.layers               = layer_ptrs.data();
    CHECK_XR_(xrEndFrame(session_, &frame_end_info));
}

void VR::Helper_::SetUpRendering_(const GFX &gfx) {
    SetUpReferenceSpace_();
    SetUpSwapchains_(GetSwapchainFormat_(GL_SRGB8_ALPHA8_EXT),
                     (XR_SWAPCHAIN_USAGE_SAMPLED_BIT |
                      XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT),
                     color_images_);
    SetUpSwapchains_(GetSwapchainFormat_(GL_DEPTH_COMPONENT16),
                     XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                     depth_images_);
    SetUpGFX_(gfx);
}

void VR::Helper_::SetUpReferenceSpace_() {
    XrPosef identity_pose{};
    identity_pose.orientation.w = 1.f;
    XrReferenceSpaceCreateInfo ps_info{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
    ps_info.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;  // Seated.
    ps_info.poseInReferenceSpace = identity_pose;
    XrSpace play_space = XR_NULL_HANDLE;
    CHECK_XR_(xrCreateReferenceSpace(session_, &ps_info, &play_space));
}

void VR::Helper_::SetUpSwapchains_(int64_t format,
                                   XrSwapchainUsageFlags usage_flags,
                                   SCImages_ &sc_images) {
    uint32_t sc_count;
    CHECK_XR_(xrEnumerateSwapchainFormats(session_, 0, &sc_count, nullptr));

    int64_t sc_formats[sc_count];
    CHECK_XR_(xrEnumerateSwapchainFormats(session_, sc_count,
                                          &sc_count, sc_formats));

    size_t view_count = views_.size();
    std::vector<XrSwapchain>               swapchains(view_count);
    std::vector<uint32_t>                  sc_lengths(view_count);
    sc_images.resize(view_count);

    for (uint32_t i = 0; i < view_count; ++i) {
        XrSwapchainCreateInfo sc_info{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
        sc_info.usageFlags  = usage_flags;
        sc_info.format      = format;
        sc_info.sampleCount = views_[i].recommendedSwapchainSampleCount;
        sc_info.width       = views_[i].recommendedImageRectWidth;
        sc_info.height      = views_[i].recommendedImageRectHeight;
        sc_info.faceCount   = 1;
        sc_info.arraySize   = 1;
        sc_info.mipCount    = 1;
        CHECK_XR_(xrCreateSwapchain(session_, &sc_info, &swapchains[i]));

        // The runtime controls how many textures we have to be able to render
        // to (e.g. "triple buffering")
        CHECK_XR_(xrEnumerateSwapchainImages(swapchains[i], 0,
                                             &sc_lengths[i], nullptr));

        std::cout << "XXXX   Swapchain " << i << " length = "
                  << sc_lengths[i] << "\n";
        sc_images[i].resize(sc_lengths[i]);
        for (uint32_t j = 0; j < sc_lengths[i]; ++j)
            sc_images[i][j].type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
        CHECK_XR_(xrEnumerateSwapchainImages(
                      swapchains[i], sc_lengths[i], &sc_lengths[i],
                      (XrSwapchainImageBaseHeader *) &sc_images[i][0]));
    }
}

void VR::Helper_::SetUpGFX_(const GFX &gfx) {
   return; // XXXX
}

void VR::Helper_::PrintInstanceProperties_() {
    XrInstanceProperties props{ XR_TYPE_INSTANCE_PROPERTIES };
    CHECK_XR_(xrGetInstanceProperties(instance_, &props));

    std::cerr << "XXXX Runtime Name:    " << props.runtimeName << "\n";
    std::cerr << "XXXX Runtime Version: "
              << XR_VERSION_MAJOR(props.runtimeVersion) << "."
              << XR_VERSION_MINOR(props.runtimeVersion) << "."
              << XR_VERSION_PATCH(props.runtimeVersion) << "\n";
}

// Returns the preferred swapchain format if it is supported, otherwise returns
// the first supported format.
int64_t VR::Helper_::GetSwapchainFormat_(int64_t preferred_format) {
    uint32_t count;
    CHECK_XR_(xrEnumerateSwapchainFormats(session_, 0, &count, nullptr));

    std::vector<int64_t> formats(count);
    CHECK_XR_(xrEnumerateSwapchainFormats(session_, count, &count, &formats[0]));

    if (std::find(formats.begin(), formats.end(),
                  preferred_format) != formats.end())
        return preferred_format;
    else
        return formats[0];
}

// Returns true if an event is available.
bool VR::Helper_::GetNextEvent_(XrEventDataBuffer &event) {
    event.type = XR_TYPE_EVENT_DATA_BUFFER;
    return xrPollEvent(instance_, &event) == XR_SUCCESS;
}

bool VR::Helper_::ProcessSessionStateChange_(
    const XrEventDataSessionStateChanged &event) {
    session_state_ = event.state;

    if (event.session != XR_NULL_HANDLE && event.session != session_) {
        Disaster_("State change for unknown session");
        return true;
    }

    bool keep_going = true;
    switch (session_state_) {
      case XR_SESSION_STATE_READY: {
          XrSessionBeginInfo info{ XR_TYPE_SESSION_BEGIN_INFO };
          info.primaryViewConfigurationType = view_type_;
          CHECK_XR_(xrBeginSession(session_, &info));
          break;
      }

      case XR_SESSION_STATE_STOPPING:
        CHECK_XR_(xrEndSession(session_));
        keep_going = false;
        break;

      case XR_SESSION_STATE_EXITING:
      case XR_SESSION_STATE_LOSS_PENDING:
        keep_going = false;
        break;

      default:
        break;
    }
    return keep_going;
}

bool VR::Helper_::RenderLayer_(
    XrTime predicted_display_time,
    const std::vector<XrCompositionLayerProjectionView> &views,
    XrCompositionLayerProjection &layer) {
    // XXXX
    return false;
}
