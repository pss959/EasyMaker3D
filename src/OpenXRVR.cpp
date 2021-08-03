#include "OpenXRVR.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#include <ion/math/matrix.h>
#include <ion/math/range.h>
#include <ion/math/rotation.h>
#include <ion/math/transformutils.h>

#include "Event.h"
#include "Interfaces/IRenderer.h"

using ion::math::Matrix4f;
using ion::math::Point2i;
using ion::math::Range2i;
using ion::math::Rotationf;
using ion::math::Vector2i;
using ion::math::Vector3f;
using ion::math::Vector4f;

// ----------------------------------------------------------------------------
// Handy macros.
// ----------------------------------------------------------------------------

//! Calls the given function and throws a VRException_ if the result indicates
//! failure.
#define CHECK_XR_(cmd) CheckXr_(cmd, #cmd, __FILE__, __LINE__)

//! Assertion checking - failure results in throwing a VRException_.
#define ASSERT_(exp) Assert_(exp, #exp, __FILE__, __LINE__)

//! Shorthand for OpenXR-required casts.
#define CAST_(to_type, var) reinterpret_cast<to_type>(var)

// ----------------------------------------------------------------------------
// Math helper functions.
// ----------------------------------------------------------------------------

//! Computes and returns an Ion viewport for the given OpenXR rectangle.
static Range2i ComputeViewportRect_(const XrRect2Di xr_rect) {
    return Range2i::BuildWithSize(
        Point2i(xr_rect.offset.x, xr_rect.offset.y),
        Vector2i(xr_rect.extent.width, xr_rect.extent.height));
}

//! Computes and returns an Ion projection matrix given an OpenXR field of
//! view and near/far Z values.
static Matrix4f ComputeProjectionMatrix_(const XrFovf &fov,
                                         float z_near, float z_far) {
    const float tan_l = tanf(fov.angleLeft);
    const float tan_r = tanf(fov.angleRight);
    const float tan_u = tanf(fov.angleUp);
    const float tan_d = tanf(fov.angleDown);

    const float tan_lr  = tan_r - tan_l;
    const float tan_du = tan_u - tan_d;

    return Matrix4f(
        2 / tan_lr, 0, (tan_r + tan_l) / tan_lr, 0,      // Row 0.
        0, 2 / tan_du, (tan_u + tan_d) / tan_du, 0,      // Row 1.
        0, 0, -(z_far + z_near) / (z_far - z_near),      // Row 2.
        -(2 * z_far * z_near) / (z_far - z_near),
        0, 0, -1, 0);                                    // Row 3.
}

//! Computes and returns an Ion view matrix given an OpenXR camera pose.
static Matrix4f ComputeViewMatrix_(const XrPosef &pose) {
    return ion::math::RotationMatrixH(
        -Rotationf::FromQuaternion(
            Vector4f(pose.orientation.x, pose.orientation.y,
                     pose.orientation.z, pose.orientation.w))) *
        ion::math::TranslationMatrix(
            -Vector3f(pose.position.x, pose.position.y, pose.position.z));
}

// ----------------------------------------------------------------------------
// OpenXRVR implementation.
// ----------------------------------------------------------------------------

OpenXRVR::OpenXRVR() {
}

OpenXRVR::~OpenXRVR() {
    try {
        for (Swapchain_ sc : swapchains_) {
            xrDestroySwapchain(sc.color.swapchain);
            xrDestroySwapchain(sc.depth.swapchain);
        }

        if (reference_space_ != XR_NULL_HANDLE)
            xrDestroySpace(reference_space_);

        if (session_ != XR_NULL_HANDLE) {
            xrEndSession(session_);
            xrDestroySession(session_);
        }
        if (instance_ != XR_NULL_HANDLE) {
            // xrDestroyInstance(instance_);  // Causes hang! (See online) XXXX
        }
    }
    catch (VRException_ &ex) {
        ReportException_(ex);
    }
}

bool OpenXRVR::Init(const Vector2i &size) {
    try {
        if (! InitInstance_())
            return false;
        PrintInstanceProperties_();
        InitSystem_();
        InitViewConfigs_();
        return true;
    }
    catch (VRException_ &ex) {
        ReportException_(ex);
        return false;
    }
}

void OpenXRVR::SetSize(const ion::math::Vector2i &new_size) {
    // Nothing to do here.
}

Vector2i OpenXRVR::GetSize() const {
    if (! view_configs_.empty()) {
        return Vector2i(view_configs_[0].recommendedImageRectWidth,
                        view_configs_[0].recommendedImageRectHeight);
    }
    else {
        return Vector2i(0, 0);
    }
}

void OpenXRVR::Render(IScene &scene, IRenderer &renderer) {
    try {
        // Initialize rendering if not already done. Do not render right away;
        // PollEvents_() has to be called to set up the session properly before
        // rendering.
        if (fb_ <= 0)
            InitRendering_(renderer);
        else
            RenderScene_(scene, renderer);
    }
    catch (VRException_ &ex) {
        ReportException_(ex);
    }
}

void OpenXRVR::EmitEvents(std::vector<Event> &events) {
    // XXXX Fix this.
    try {
        PollEvents_(events);
    }
    catch (VRException_ &ex) {
        ReportException_(ex);
    }
}

bool OpenXRVR::HandleEvent(const Event &event) {
    // XXXX Nothing yet.
    return false;
}

// ----------------------------------------------------------------------------
// VR::Helper_ Initialization subfunctions.
// ----------------------------------------------------------------------------

bool OpenXRVR::InitInstance_() {
    const char *extension = XR_KHR_OPENGL_ENABLE_EXTENSION_NAME;

    XrInstanceCreateInfo create_info{ XR_TYPE_INSTANCE_CREATE_INFO };
    create_info.enabledExtensionCount = 1;
    create_info.enabledExtensionNames = &extension;
    strcpy(create_info.applicationInfo.applicationName, "VR Test");
    create_info.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
    return XR_SUCCEEDED(xrCreateInstance(&create_info, &instance_));
}

void OpenXRVR::InitSystem_() {
    ASSERT_(instance_);
    XrFormFactor form_factor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrSystemGetInfo system_get_info{ XR_TYPE_SYSTEM_GET_INFO };
    system_get_info.formFactor = form_factor;
    CHECK_XR_(xrGetSystem(instance_, &system_get_info, &system_id_));

    // OpenXR requires the graphics requirements to be queried in order for the
    // device to be validated.
    PFN_xrGetOpenGLGraphicsRequirementsKHR reqs_pfn = nullptr;
    CHECK_XR_(xrGetInstanceProcAddr(
                  instance_, "xrGetOpenGLGraphicsRequirementsKHR",
                  CAST_(PFN_xrVoidFunction *, &reqs_pfn)));
    XrGraphicsRequirementsOpenGLKHR reqs;
              reqs.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR;
    CHECK_XR_(reqs_pfn(instance_, system_id_, &reqs));

#if XXXX
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    const XrVersion version = XR_MAKE_VERSION(major, minor, 0);
    if (reqs.minApiVersionSupported > version) {
        std::ostringstream out;
        out << "Required runtime OpenGL version ("
            << reqs.minApiVersionSupported
            << ") newer than OpenGL library version ("
            << version << ")";
        Throw_(out.str());
    }
#endif
}

void OpenXRVR::InitViewConfigs_() {
    ASSERT_(instance_  != XR_NULL_HANDLE);
    ASSERT_(system_id_ != XR_NULL_SYSTEM_ID);

    // Get the number of views. (Should be 2 for stereo.)
    uint32_t view_count = 0;
    CHECK_XR_(xrEnumerateViewConfigurationViews(
                  instance_, system_id_, view_type_, 0,
                  &view_count, nullptr));
    if (view_count == 0)
        Throw_("No view configurations available");

    // Get the view configurations.
    view_configs_.resize(view_count, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
    CHECK_XR_(xrEnumerateViewConfigurationViews(
                  instance_, system_id_, view_type_, view_count,
                  &view_count, view_configs_.data()));
}

void OpenXRVR::InitRendering_(IRenderer &renderer) {
    fb_ = renderer.CreateFramebuffer();
    ASSERT_(fb_ > 0);

    InitViews_();
    InitSession_(renderer);
    InitReferenceSpace_();
    InitSwapchains_();
    InitProjectionViews_();
}

void OpenXRVR::InitViews_() {
    ASSERT_(instance_  != XR_NULL_HANDLE);
    ASSERT_(system_id_ != XR_NULL_SYSTEM_ID);

    // Get the number of views. (Should be 2 for stereo.)
    uint32_t view_count = 0;
    CHECK_XR_(xrEnumerateViewConfigurationViews(
                  instance_, system_id_, view_type_, 0,
                  &view_count, nullptr));
    if (view_count == 0)
        Throw_("No view configurations available");

    // Get the view configurations.
    view_configs_.resize(view_count, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
    CHECK_XR_(xrEnumerateViewConfigurationViews(
                  instance_, system_id_, view_type_, view_count,
                  &view_count, view_configs_.data()));

    views_.resize(view_count, { XR_TYPE_VIEW });
}

void OpenXRVR::InitSession_(IRenderer &renderer) {
    ASSERT_(instance_  != XR_NULL_HANDLE);
    ASSERT_(system_id_ != XR_NULL_SYSTEM_ID);

    XrGraphicsBindingOpenGLXlibKHR binding {
        XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR };
    binding.xDisplay    = renderer.GetDisplay();
    binding.glxDrawable = renderer.GetDrawable();
    binding.glxContext  = renderer.GetContext();

    XrSessionCreateInfo info{ XR_TYPE_SESSION_CREATE_INFO };
    info.systemId = system_id_;
    info.next     = &binding;
    CHECK_XR_(xrCreateSession(instance_, &info, &session_));
}

void OpenXRVR::InitReferenceSpace_() {
    ASSERT_(session_ != XR_NULL_HANDLE);

    XrPosef identity_pose{};
    identity_pose.orientation.w = 1.f;

    XrReferenceSpaceCreateInfo info{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
    info.referenceSpaceType   = XR_REFERENCE_SPACE_TYPE_LOCAL;  // Seated.
    info.poseInReferenceSpace = identity_pose;

    CHECK_XR_(xrCreateReferenceSpace(session_, &info, &reference_space_));
}

void OpenXRVR::InitSwapchains_() {
    ASSERT_(session_ != XR_NULL_HANDLE);
    ASSERT_(! view_configs_.empty());

    // Create a swapchain for each view.
    swapchains_.resize(view_configs_.size());

    const int64_t color_format = GetSwapchainFormat_(GL_SRGB8_ALPHA8_EXT);
    const int64_t depth_format = GetSwapchainFormat_(GL_DEPTH_COMPONENT16);

    XrSwapchainCreateInfo info{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
    info.faceCount = 1;
    info.arraySize = 1;
    info.mipCount  = 1;

    for (size_t i = 0; i < swapchains_.size(); ++i) {
        Swapchain_ &sc = swapchains_[i];

        info.sampleCount = view_configs_[i].recommendedSwapchainSampleCount;
        info.width       = view_configs_[i].recommendedImageRectWidth;
        info.height      = view_configs_[i].recommendedImageRectHeight;

        // Color swapchain.
        info.usageFlags = (XR_SWAPCHAIN_USAGE_SAMPLED_BIT |
                           XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT);
        info.format     = color_format;
        CHECK_XR_(xrCreateSwapchain(session_, &info, &sc.color.swapchain));

        // Depth swapchain.
        info.usageFlags = XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        info.format     = depth_format;
        CHECK_XR_(xrCreateSwapchain(session_, &info, &sc.depth.swapchain));

        // Images.
        uint32_t image_count;
        CHECK_XR_(xrEnumerateSwapchainImages(sc.color.swapchain, 0,
                                             &image_count, nullptr));
        InitImages_(sc.color, image_count);
        InitImages_(sc.depth, image_count);
    }
}

void OpenXRVR::InitProjectionViews_() {
    projection_views_.resize(view_configs_.size());
    depth_infos_.resize(view_configs_.size());

    for (size_t i = 0; i < projection_views_.size(); ++i) {
        XrCompositionLayerProjectionView &proj_view = projection_views_[i];

        proj_view = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
        proj_view.subImage.swapchain = swapchains_[i].color.swapchain;
        proj_view.subImage.imageRect.offset = { 0, 0 };
        proj_view.subImage.imageRect.extent.width =
            view_configs_[i].recommendedImageRectWidth;
        proj_view.subImage.imageRect.extent.height =
            view_configs_[i].recommendedImageRectHeight;
        // pose and fov will be updated when rendering every frame.

        // Set up depth info and chain it.
        XrCompositionLayerDepthInfoKHR &depth_info = depth_infos_[i];
        depth_info = { XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR };
        depth_info.next = NULL;
        depth_info.minDepth = 0.f;
        depth_info.maxDepth = 1.f;
        depth_info.nearZ = kZNear;
        depth_info.farZ  = kZFar;
        depth_info.subImage.swapchain = swapchains_[i].depth.swapchain;
        depth_info.subImage.imageArrayIndex = 0;
        depth_info.subImage.imageRect.offset.x = 0;
        depth_info.subImage.imageRect.offset.y = 0;
        depth_info.subImage.imageRect.extent.width =
            view_configs_[i].recommendedImageRectWidth;
        depth_info.subImage.imageRect.extent.height =
            view_configs_[i].recommendedImageRectHeight;

        proj_view.next = &depth_info;
    }
}

// ----------------------------------------------------------------------------
// Other VR::Helper_ private functions.
// ----------------------------------------------------------------------------

void OpenXRVR::PrintInstanceProperties_() {
    ASSERT_(instance_ != XR_NULL_HANDLE);

    XrInstanceProperties props{ XR_TYPE_INSTANCE_PROPERTIES };
    CHECK_XR_(xrGetInstanceProperties(instance_, &props));

    std::cout << "==== Runtime Name:    " << props.runtimeName << "\n";
    std::cout << "==== Runtime Version: "
              << XR_VERSION_MAJOR(props.runtimeVersion) << "."
              << XR_VERSION_MINOR(props.runtimeVersion) << "."
              << XR_VERSION_PATCH(props.runtimeVersion) << "\n";
}

// Returns the preferred swapchain format if it is supported, otherwise returns
// the first supported format.
int64_t OpenXRVR::GetSwapchainFormat_(int64_t preferred_format) {
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

void OpenXRVR::InitImages_(Swapchain_::SC_ &sc, uint32_t count) {
    ASSERT_(count > 0);

    sc.gl_images.resize(count, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR });
    sc.images.resize(count);
    for (uint32_t j = 0; j < count; ++j)
        sc.images[j] = CAST_(XrSwapchainImageBaseHeader *, &sc.gl_images[j]);
    CHECK_XR_(xrEnumerateSwapchainImages(sc.swapchain, count,
                                         &count, sc.images[0]));
}

void OpenXRVR::PollEvents_(std::vector<Event> &events) {
    // Process all pending messages.
    bool keep_going = true;
    XrEventDataBuffer event;
    while (keep_going && GetNextEvent_(event)) {
        switch (event.type) {
          case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
            ReportDisaster_("OpenXR instance loss pending");
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
    if (! keep_going) {
        Event ev;
        ev.flags = static_cast<uint32_t>(Event::Flag::kExit);
        events.push_back(ev);
    }
}

// Returns true if an event is available.
bool OpenXRVR::GetNextEvent_(XrEventDataBuffer &event) {
    event.type = XR_TYPE_EVENT_DATA_BUFFER;
    return xrPollEvent(instance_, &event) == XR_SUCCESS;
}

bool OpenXRVR::ProcessSessionStateChange_(
    const XrEventDataSessionStateChanged &event) {
    session_state_ = event.state;

    if (event.session != XR_NULL_HANDLE && event.session != session_) {
        ReportDisaster_("State change for unknown session");
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

void OpenXRVR::RenderScene_(IScene &scene, IRenderer &renderer) {
    ASSERT_(session_ != XR_NULL_HANDLE);

    XrFrameWaitInfo wait_info{XR_TYPE_FRAME_WAIT_INFO};
    XrFrameState    frame_state{XR_TYPE_FRAME_STATE};
    CHECK_XR_(xrWaitFrame(session_, &wait_info, &frame_state));
    if (frame_state.shouldRender != XR_TRUE)
        return;

    XrFrameBeginInfo frame_begin_info{ XR_TYPE_FRAME_BEGIN_INFO };
    CHECK_XR_(xrBeginFrame(session_, &frame_begin_info));

    if (RenderViews_(scene, renderer, frame_state.predictedDisplayTime)) {
        XrCompositionLayerProjection layer_proj{
            XR_TYPE_COMPOSITION_LAYER_PROJECTION };
        layer_proj.space     = reference_space_;
        layer_proj.viewCount = (uint32_t) projection_views_.size();
        layer_proj.views     = projection_views_.data();

        const XrCompositionLayerBaseHeader* submitted_layer =
            CAST_(XrCompositionLayerBaseHeader *, &layer_proj);

        XrFrameEndInfo frame_end_info{ XR_TYPE_FRAME_END_INFO };
        frame_end_info.displayTime          = frame_state.predictedDisplayTime;
        frame_end_info.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
        frame_end_info.layerCount           = 1;
        frame_end_info.layers               = &submitted_layer;
        CHECK_XR_(xrEndFrame(session_, &frame_end_info));
    }
}

bool OpenXRVR::RenderViews_(IScene &scene, IRenderer &renderer,
                            XrTime predicted_time) {
    ASSERT_(! view_configs_.empty());
    ASSERT_(! projection_views_.empty());

    XrViewState view_state{ XR_TYPE_VIEW_STATE };
    uint32_t view_capacity_input = (uint32_t) views_.size();
    uint32_t view_count_output;

    XrViewLocateInfo view_locate_info{ XR_TYPE_VIEW_LOCATE_INFO };
    view_locate_info.viewConfigurationType = view_type_;
    view_locate_info.displayTime           = predicted_time;
    view_locate_info.space                 = reference_space_;

    CHECK_XR_(xrLocateViews(session_, &view_locate_info, &view_state,
                            view_capacity_input, &view_count_output,
                            views_.data()));
    if (! (view_state.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) ||
        ! (view_state.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT))
        return false;  // No valid tracking poses for the views.

    ASSERT_(view_count_output == view_capacity_input);
    ASSERT_(view_count_output == view_configs_.size());
    ASSERT_(view_count_output == swapchains_.size());

    // Render view to the appropriate part of the swapchain image.
    for (size_t i = 0; i < view_configs_.size(); ++i) {
        XrSwapchain color_swapchain = swapchains_[i].color.swapchain;
        XrSwapchain depth_swapchain = swapchains_[i].depth.swapchain;

        XrSwapchainImageAcquireInfo color_acquire_info{
            XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
        XrSwapchainImageAcquireInfo depth_acquire_info{
            XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
        uint32_t color_index, depth_index;
        CHECK_XR_(xrAcquireSwapchainImage(color_swapchain, &color_acquire_info,
                                          &color_index));
        CHECK_XR_(xrAcquireSwapchainImage(depth_swapchain, &depth_acquire_info,
                                          &depth_index));

        XrSwapchainImageWaitInfo wait_info{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
        wait_info.timeout = XR_INFINITE_DURATION;
        CHECK_XR_(xrWaitSwapchainImage(color_swapchain, &wait_info));
        CHECK_XR_(xrWaitSwapchainImage(depth_swapchain, &wait_info));

        projection_views_[i].pose = views_[i].pose;
        projection_views_[i].fov  = views_[i].fov;

        RenderView_(scene, renderer, i, color_index, depth_index);

        XrSwapchainImageReleaseInfo release_info{
            XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
        CHECK_XR_(xrReleaseSwapchainImage(color_swapchain, &release_info));
        CHECK_XR_(xrReleaseSwapchainImage(depth_swapchain, &release_info));
    }

    return true;
}

void OpenXRVR::RenderView_(IScene &scene, IRenderer &renderer,
                           int view_index, int color_index, int depth_index) {
    ASSERT_(fb_ > 0);

    const Swapchain_ &swapchain = swapchains_[view_index];
    ASSERT_((size_t) color_index < swapchain.color.gl_images.size());
    ASSERT_((size_t) depth_index < swapchain.depth.gl_images.size());

    const auto &proj_view = projection_views_[view_index];

    // Set up the IRenderer::Target.
    IRenderer::Target targ;
    targ.viewport          = ComputeViewportRect_(proj_view.subImage.imageRect);
    targ.projection_matrix = ComputeProjectionMatrix_(proj_view.fov,
                                                      kZNear, kZFar);
    targ.view_matrix       = ComputeViewMatrix_(proj_view.pose);
    targ.target_fb         = fb_;
    targ.color_fb          = swapchain.color.gl_images[color_index].image;
    targ.depth_fb          = swapchain.depth.gl_images[depth_index].image;

    renderer.RenderSceneToTarget(scene, targ);
}

void OpenXRVR::CheckXr_(XrResult res, const char *cmd,
                        const char *file, int line) {
    // std::cout << "==== <" << cmd << ">\n"; // Uncomment for tracing.
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

void OpenXRVR::Assert_(bool exp, const char *expstr,
                          const char *file, int line) {
    if (! exp) {
        std::ostringstream out;
        out << "***Assertion failure: <" << expstr
            << "> at " << file << ":" << line;
        Throw_(out.str());
    }
}

void OpenXRVR::Throw_(const std::string &msg) {
    // std::cerr << "**************** " << msg << "\n";
    throw VRException_(msg);
}

void OpenXRVR::ReportException_(const VRException_ &ex) {
    std::cerr << ex.what() << "\n";
}

void OpenXRVR::ReportDisaster_(const char *msg) {
    std::cerr << "*** " << msg << ": Expect disaster\n";
}
