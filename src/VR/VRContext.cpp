#include "VR/VRContext.h"

#include <iostream>

#include <ion/math/transformutils.h>

#include "Event.h"
#include "Interfaces/IRenderer.h"
#include "Util/General.h"
#include "Util/OutputMuter.h"
#include "VR/VRStructs.h"

// ----------------------------------------------------------------------------
// VRContext implementation.
// ----------------------------------------------------------------------------

VRContext::VRContext() {
}

VRContext::~VRContext() {
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
            xrDestroyInstance(instance_);
        }
    }
    catch (VRException_ &ex) {
        ReportException_(ex);
    }
}

bool VRContext::Init() {
    try {
        // Use an OutputMuter around InitInstance_() so that error messages are
        // not spewed when OpenXR does not detect a device.
        {
            Util::OutputMuter muter;
            if (! InitInstance_())
                return false;
        }
        VRBase::SetInstance(instance_);  // Let base class have it.
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

void VRContext::Render(const SG::Scene &scene, IRenderer &renderer,
                       const Point3f &base_position) {
    try {
        // Initialize rendering if not already done. Do not render right away;
        // PollEvents_() has to be called to set up the session properly before
        // rendering.
        if (fb_ <= 0)
            InitRendering_(renderer);
        else
            Render_(scene, renderer, base_position);
    }
    catch (VRException_ &ex) {
        ReportException_(ex);
    }
}

// ----------------------------------------------------------------------------
// VR::Helper_ Initialization subfunctions.
// ----------------------------------------------------------------------------

bool VRContext::InitInstance_() {
    const char *extension = XR_KHR_OPENGL_ENABLE_EXTENSION_NAME;

    XrInstanceCreateInfo create_info = VRS::BuildInstanceCreateInfo();
    create_info.enabledExtensionCount = 1;
    create_info.enabledExtensionNames = &extension;
    strcpy(create_info.applicationInfo.applicationName, "VR Test");
    create_info.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
    return XR_SUCCEEDED(xrCreateInstance(&create_info, &instance_));
}

void VRContext::InitSystem_() {
    ASSERT_(instance_);
    XrSystemGetInfo system_get_info = VRS::BuildSystemGetInfo();
    system_get_info.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    CHECK_XR_(xrGetSystem(instance_, &system_get_info, &system_id_));

    // OpenXR requires the graphics requirements to be queried in order for the
    // device to be validated.
    PFN_xrGetOpenGLGraphicsRequirementsKHR reqs_pfn = nullptr;
    CHECK_XR_(xrGetInstanceProcAddr(
                  instance_, "xrGetOpenGLGraphicsRequirementsKHR",
                  CAST_(PFN_xrVoidFunction *, &reqs_pfn)));
    XrGraphicsRequirementsOpenGLKHR reqs =
        VRS::BuildGraphicsRequirementsOpenGLKHR();
    CHECK_XR_(reqs_pfn(instance_, system_id_, &reqs));

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
}

void VRContext::InitViewConfigs_() {
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
    XrViewConfigurationView config_view = VRS::BuildViewConfigurationView();
    view_configs_.resize(view_count, config_view);
    CHECK_XR_(xrEnumerateViewConfigurationViews(
                  instance_, system_id_, view_type_, view_count,
                  &view_count, view_configs_.data()));
}

void VRContext::InitRendering_(IRenderer &renderer) {
    fb_ = renderer.CreateFramebuffer();
    ASSERT_(fb_ > 0);

    InitViews_();
    InitSession_(renderer);
    InitReferenceSpace_();
    InitSwapchains_();
    InitProjectionViews_();
}

void VRContext::InitViews_() {
    ASSERT_(instance_  != XR_NULL_HANDLE);
    ASSERT_(system_id_ != XR_NULL_SYSTEM_ID);

    XrView view = VRS::BuildView();
    views_.resize(view_configs_.size(), view);
}

void VRContext::InitSession_(IRenderer &renderer) {
    ASSERT_(instance_  != XR_NULL_HANDLE);
    ASSERT_(system_id_ != XR_NULL_SYSTEM_ID);

    XrGraphicsBindingOpenGLXlibKHR binding =
        VRS::BuildGraphicsBindingOpenGLXlibKHR();
    binding.xDisplay    = renderer.GetDisplay();
    binding.glxDrawable = renderer.GetDrawable();
    binding.glxContext  = renderer.GetContext();

    XrSessionCreateInfo info = VRS::BuildSessionCreateInfo();
    info.systemId = system_id_;
    info.next     = &binding;
    CHECK_XR_(xrCreateSession(instance_, &info, &session_));
}

void VRContext::InitReferenceSpace_() {
    ASSERT_(session_ != XR_NULL_HANDLE);

    XrPosef identity_pose = VRS::BuildPosef();

    XrReferenceSpaceCreateInfo info = VRS::BuildReferenceSpaceCreateInfo();
    info.referenceSpaceType   = XR_REFERENCE_SPACE_TYPE_LOCAL;  // Seated.
    info.poseInReferenceSpace = identity_pose;

    CHECK_XR_(xrCreateReferenceSpace(session_, &info, &reference_space_));
}

void VRContext::InitSwapchains_() {
    ASSERT_(session_ != XR_NULL_HANDLE);
    ASSERT_(! view_configs_.empty());

    // Create a swapchain for each view.
    swapchains_.resize(view_configs_.size());

    const int64_t color_format = GetSwapchainFormat_(GL_SRGB8_ALPHA8_EXT);
    const int64_t depth_format = GetSwapchainFormat_(GL_DEPTH_COMPONENT16);

    XrSwapchainCreateInfo info = VRS::BuildSwapchainCreateInfo();
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

void VRContext::InitProjectionViews_() {
    projection_views_.resize(view_configs_.size());
    depth_infos_.resize(view_configs_.size());

    for (size_t i = 0; i < projection_views_.size(); ++i) {
        XrCompositionLayerProjectionView &proj_view = projection_views_[i];
        proj_view = VRS::BuildCompositionLayerProjectionView();
        proj_view.subImage.swapchain = swapchains_[i].color.swapchain;
        proj_view.subImage.imageRect.offset = { 0, 0 };
        proj_view.subImage.imageRect.extent.width =
            view_configs_[i].recommendedImageRectWidth;
        proj_view.subImage.imageRect.extent.height =
            view_configs_[i].recommendedImageRectHeight;
        // pose and fov will be updated when rendering every frame.

        // Set up depth info and chain it.
        XrCompositionLayerDepthInfoKHR &depth_info = depth_infos_[i];
        depth_info = VRS::BuildCompositionLayerDepthInfoKHR();
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

void VRContext::PrintInstanceProperties_() {
    ASSERT_(instance_ != XR_NULL_HANDLE);

    XrInstanceProperties props = VRS::BuildInstanceProperties();
    CHECK_XR_(xrGetInstanceProperties(instance_, &props));

    std::cout << "==== Runtime Name:    " << props.runtimeName << "\n";
    std::cout << "==== Runtime Version: "
              << XR_VERSION_MAJOR(props.runtimeVersion) << "."
              << XR_VERSION_MINOR(props.runtimeVersion) << "."
              << XR_VERSION_PATCH(props.runtimeVersion) << "\n";
}

// Returns the preferred swapchain format if it is supported, otherwise returns
// the first supported format.
int64_t VRContext::GetSwapchainFormat_(int64_t preferred_format) {
    uint32_t count;
    CHECK_XR_(xrEnumerateSwapchainFormats(session_, 0, &count, nullptr));

    std::vector<int64_t> formats(count);
    CHECK_XR_(xrEnumerateSwapchainFormats(session_, count, &count, &formats[0]));

    if (Util::Contains(formats, preferred_format))
        return preferred_format;
    else
        return formats[0];
}

void VRContext::InitImages_(Swapchain_::SC_ &sc, uint32_t count) {
    ASSERT_(count > 0);

    XrSwapchainImageOpenGLKHR image = VRS::BuildSwapchainImageOpenGLKHR();
    sc.gl_images.resize(count, image);
    sc.images.resize(count);
    for (uint32_t j = 0; j < count; ++j)
        sc.images[j] = CAST_(XrSwapchainImageBaseHeader *, &sc.gl_images[j]);
    CHECK_XR_(xrEnumerateSwapchainImages(sc.swapchain, count,
                                         &count, sc.images[0]));
}

void VRContext::Render_(const SG::Scene &scene, IRenderer &renderer,
                        const Point3f &base_position) {
    ASSERT_(session_ != XR_NULL_HANDLE);

    XrFrameWaitInfo wait_info   = VRS::BuildFrameWaitInfo();
    XrFrameState    frame_state = VRS::BuildFrameState();
    CHECK_XR_(xrWaitFrame(session_, &wait_info, &frame_state));
    if (frame_state.shouldRender != XR_TRUE)
        return;

    time_ = frame_state.predictedDisplayTime;

    XrFrameBeginInfo frame_begin_info = VRS::BuildFrameBeginInfo();
    CHECK_XR_(xrBeginFrame(session_, &frame_begin_info));

    if (RenderViews_(scene, renderer, base_position)) {
        XrCompositionLayerProjection layer_proj =
            VRS::BuildCompositionLayerProjection();
        layer_proj.space     = reference_space_;
        layer_proj.viewCount = static_cast<uint32_t>(projection_views_.size());
        layer_proj.views     = projection_views_.data();

        const XrCompositionLayerBaseHeader* submitted_layer =
            CAST_(XrCompositionLayerBaseHeader *, &layer_proj);

        XrFrameEndInfo frame_end_info = VRS::BuildFrameEndInfo();
        frame_end_info.displayTime          = time_;
        frame_end_info.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
        frame_end_info.layerCount           = 1;
        frame_end_info.layers               = &submitted_layer;
        CHECK_XR_(xrEndFrame(session_, &frame_end_info));
    }
}

bool VRContext::RenderViews_(const SG::Scene &scene, IRenderer &renderer,
                             const Point3f &base_position) {
    ASSERT_(! view_configs_.empty());
    ASSERT_(! projection_views_.empty());

    uint32_t view_capacity_input = static_cast<uint32_t>(views_.size());
    uint32_t view_count_output;

    XrViewLocateInfo view_locate_info = VRS::BuildViewLocateInfo();
    view_locate_info.viewConfigurationType = view_type_;
    view_locate_info.displayTime           = time_;
    view_locate_info.space                 = reference_space_;

    XrViewState view_state = VRS::BuildViewState();
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

        XrSwapchainImageAcquireInfo color_acquire_info =
            VRS::BuildSwapchainImageAcquireInfo();
        XrSwapchainImageAcquireInfo depth_acquire_info =
            VRS::BuildSwapchainImageAcquireInfo();
        uint32_t color_index, depth_index;
        CHECK_XR_(xrAcquireSwapchainImage(color_swapchain, &color_acquire_info,
                                          &color_index));
        CHECK_XR_(xrAcquireSwapchainImage(depth_swapchain, &depth_acquire_info,
                                          &depth_index));

        XrSwapchainImageWaitInfo wait_info =
            VRS::BuildSwapchainImageWaitInfo();
        wait_info.timeout = XR_INFINITE_DURATION;
        CHECK_XR_(xrWaitSwapchainImage(color_swapchain, &wait_info));
        CHECK_XR_(xrWaitSwapchainImage(depth_swapchain, &wait_info));

        projection_views_[i].pose = views_[i].pose;
        projection_views_[i].fov  = views_[i].fov;

        RenderView_(scene, renderer, base_position,
                    i, color_index, depth_index);

        XrSwapchainImageReleaseInfo release_info =
            VRS::BuildSwapchainImageReleaseInfo();
        CHECK_XR_(xrReleaseSwapchainImage(color_swapchain, &release_info));
        CHECK_XR_(xrReleaseSwapchainImage(depth_swapchain, &release_info));
    }

    return true;
}

void VRContext::RenderView_(const SG::Scene &scene, IRenderer &renderer,
                            const Point3f &base_position, int view_index,
                            int color_index, int depth_index) {
    ASSERT_(fb_ > 0);

    const Swapchain_ &swapchain = swapchains_[view_index];
    ASSERT_(static_cast<size_t>(color_index) <
            swapchain.color.gl_images.size());
    ASSERT_(static_cast<size_t>(depth_index) <
            swapchain.depth.gl_images.size());

    const auto &proj_view = projection_views_[view_index];

    // Set up a Frustum for viewing.
    Frustum frustum;
    frustum.viewport    = ToRange2i(proj_view.subImage.imageRect);
    frustum.position    = ToVector3f(proj_view.pose.position) + base_position;
    frustum.orientation = ToRotationf(proj_view.pose.orientation);
    frustum.fov_left    = Anglef::FromRadians(proj_view.fov.angleLeft);
    frustum.fov_right   = Anglef::FromRadians(proj_view.fov.angleRight);
    frustum.fov_up      = Anglef::FromRadians(proj_view.fov.angleUp);
    frustum.fov_down    = Anglef::FromRadians(proj_view.fov.angleDown);
    frustum.near        = kZNear;
    frustum.far         = kZFar;

    // Set up the IRenderer::FBTarget.
    IRenderer::FBTarget target;
    target.target_fb = fb_;
    target.color_fb  = swapchain.color.gl_images[color_index].image;
    target.depth_fb  = swapchain.depth.gl_images[depth_index].image;

    renderer.RenderScene(scene, frustum, &target);
}
