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
    void InitInstance();
    void InitSystem();
    void InitViewConfigs();
    void InitSession(const GFX &gfx);
    void InitReferenceSpace();
    void InitSwapchains(const GFX &gfx);
    void InitGraphics(const GFX &gfx);

    // Event handling.
    bool PollEvents();

    // Query.
    int GetWidth() const {
        return view_configs_[0].recommendedImageRectWidth;
    }
    int GetHeight() const {
        return view_configs_[0].recommendedImageRectHeight;
    }

    // Rendering.
    void Draw(const GFX &gfx);

  private:
    // Stores information for each XrSwapchain.
    struct Swapchain_ {
        struct SC_ {
            XrSwapchain                               swapchain;
            std::vector<XrSwapchainImageOpenGLKHR>    gl_images;
            std::vector<XrSwapchainImageBaseHeader *> images;
        };
        SC_ color;
        SC_ depth;
    };

    typedef std::vector<Swapchain_>                       Swapchains_;
    typedef std::vector<XrCompositionLayerProjectionView> LayerViews_;
    typedef std::vector<XrView>                           Views_;
    typedef std::vector<XrViewConfigurationView>          ViewConfigs_;

    const XrViewConfigurationType view_type_ =
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    XrInstance     instance_        = nullptr;
    XrSystemId     system_id_       = XR_NULL_SYSTEM_ID;
    XrSession      session_         = XR_NULL_HANDLE;
    XrSessionState session_state_   = XR_SESSION_STATE_UNKNOWN;
    XrSpace        reference_space_ = XR_NULL_HANDLE;

    Swapchains_    swapchains_;
    ViewConfigs_   view_configs_;
    Views_         views_;

    // Helpers.
    void    PrintInstanceProperties_();
    int64_t GetSwapchainFormat_(int64_t preferred);
    void    InitImages_(Swapchain_::SC_ &sc, uint32_t count);
    bool    GetNextEvent_(XrEventDataBuffer &event);
    bool    ProcessSessionStateChange_(
        const XrEventDataSessionStateChanged &event);
    bool    RenderLayer_(XrTime predicted_display_time,
                         const LayerViews_ &layer_views,
                         XrCompositionLayerProjection &layer);

    // Error checking and reporting.
    void CheckXr_(XrResult res, const char *cmd, const char *file, int line);
    void Disaster_(const char *msg);
};

// ----------------------------------------------------------------------------
// VR class.
// ----------------------------------------------------------------------------

VR::VR() : helper_(new Helper_()) {
}

VR::~VR() {
}

void VR::Init() {
    helper_->InitInstance();
    helper_->InitSystem();
    helper_->InitViewConfigs();
}

int VR::GetWidth() {
    return helper_->GetWidth();
}

int VR::GetHeight() {
    return helper_->GetHeight();
}

void VR::InitGraphics(const GFX &gfx) {
    helper_->InitSession(gfx);
    helper_->InitReferenceSpace();
    helper_->InitSwapchains(gfx);
}

bool VR::PollEvents() {
    return helper_->PollEvents();
}

void VR::Draw(const GFX &gfx) {
    helper_->Draw(gfx);
}

// ----------------------------------------------------------------------------
// VR::Helper_ public functions.
// ----------------------------------------------------------------------------

VR::Helper_::~Helper_() {
    for (Swapchain_ sc : swapchains_) {
        xrDestroySwapchain(sc.color.swapchain);
        xrDestroySwapchain(sc.depth.swapchain);
    }

    if (session_ != XR_NULL_HANDLE) {
        xrEndSession(session_);
        xrDestroySession(session_);
    }
    if (instance_ != XR_NULL_HANDLE) {
        // xrDestroyInstance(instance_);  // Causes hang! (See online) XXXX
    }
}

void VR::Helper_::InitInstance() {
    const char * extension = XR_KHR_OPENGL_ENABLE_EXTENSION_NAME;

    XrInstanceCreateInfo create_info{ XR_TYPE_INSTANCE_CREATE_INFO };
    create_info.enabledExtensionCount = 1;
    create_info.enabledExtensionNames = &extension;
    strcpy(create_info.applicationInfo.applicationName, "VR Test");
    create_info.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
    CHECK_XR_(xrCreateInstance(&create_info, &instance_));

    PrintInstanceProperties_();
}

void VR::Helper_::InitSystem() {
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
    std::cout << "XXXX OpenGL Version:  " << glGetString(GL_VERSION)  << "\n";
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::cerr << "XXXX GL Version " << major << " / " << minor << "\n";
    const XrVersion version = XR_MAKE_VERSION(major, minor, 0);
    if (reqs.minApiVersionSupported > version) {
        std::ostringstream out;
        out << "Required runtime OpenGL version ("
            << reqs.minApiVersionSupported
            << ") newer than OpenGL library version ("
            << version << ")";
        throw VR::VRException(out.str());
    }
#endif
}

void VR::Helper_::InitViewConfigs() {
    // Get the number of views. (Should be 2 for stereo.)
    uint32_t view_count = 0;
    CHECK_XR_(xrEnumerateViewConfigurationViews(
                  instance_, system_id_, view_type_, 0,
                  &view_count, nullptr));
    if (view_count == 0)
        throw VR::VRException("No view configurations available");

    // Get the view configurations.
    view_configs_.resize(view_count, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
    CHECK_XR_(xrEnumerateViewConfigurationViews(
                  instance_, system_id_, view_type_, view_count,
                  &view_count, view_configs_.data()));
}

void VR::Helper_::InitSession(const GFX &gfx) {
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
}

void VR::Helper_::InitReferenceSpace() {
    XrPosef identity_pose{};
    identity_pose.orientation.w = 1.f;

    XrReferenceSpaceCreateInfo info{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
    info.referenceSpaceType   = XR_REFERENCE_SPACE_TYPE_LOCAL;  // Seated.
    info.poseInReferenceSpace = identity_pose;

    CHECK_XR_(xrCreateReferenceSpace(session_, &info, &reference_space_));
}

void VR::Helper_::InitSwapchains(const GFX &gfx) {
    // Get the number of views. (Should be 2 for stereo.)
    uint32_t view_count = 0;
    CHECK_XR_(xrEnumerateViewConfigurationViews(
                  instance_, system_id_, view_type_, 0,
                  &view_count, nullptr));
    if (view_count == 0)
        throw VR::VRException("No view configurations available");

    // Get the view configurations.
    view_configs_.resize(view_count, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
    CHECK_XR_(xrEnumerateViewConfigurationViews(
                  instance_, system_id_, view_type_, view_count,
                  &view_count, view_configs_.data()));

    // Create a swapchain for each view.
    swapchains_.resize(view_count);

    const int64_t color_format = GetSwapchainFormat_(GL_SRGB8_ALPHA8_EXT);
    const int64_t depth_format = GetSwapchainFormat_(GL_DEPTH_COMPONENT16);

    XrSwapchainCreateInfo info{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
    info.faceCount = 1;
    info.arraySize = 1;
    info.mipCount  = 1;

    for (uint32_t i = 0; i < view_count; ++i) {
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

// ----------------------------------------------------------------------------
// VR::Helper_ private functions.
// ----------------------------------------------------------------------------

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

void VR::Helper_::InitImages_(Swapchain_::SC_ &sc, uint32_t count) {
    sc.gl_images.resize(count, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR });
    sc.images.resize(count);
    for (uint32_t j = 0; j < count; ++j)
        sc.images[j] = CAST_(XrSwapchainImageBaseHeader *, &sc.gl_images[j]);
    CHECK_XR_(xrEnumerateSwapchainImages(sc.swapchain, count,
                                         &count, sc.images[0]));
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

#if XXXX
    // Render view to the appropriate part of the swapchain image.
    for (size_t i = 0; i < view_configs_.size(); ++i) {
        XrSwapchain swapchain = color_swapchains_[i];

        XrSwapchainImageAcquireInfo acquire_info;
        acquire_info.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
        uint32_t index;
        CHECK_XR_(xrAcquireSwapchainImage(swapchain, &acquire_info, &index));

        XrSwapchainImageWaitInfo wait_info{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
        wait_info.timeout = XR_INFINITE_DURATION;
        CHECK_XR_(xrWaitSwapchainImage(swapchain, &wait_info));

        projectionLayerViews[i] = {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW};
        projectionLayerViews[i].pose = m_views[i].pose;
        projectionLayerViews[i].fov = m_views[i].fov;
        projectionLayerViews[i].subImage.swapchain = viewSwapchain.handle;
        projectionLayerViews[i].subImage.imageRect.offset = {0, 0};
        projectionLayerViews[i].subImage.imageRect.extent = {viewSwapchain.width, viewSwapchain.height};

        const XrSwapchainImageBaseHeader* const swapchainImage = m_swapchainImages[viewSwapchain.handle][swapchainImageIndex];
        m_graphicsPlugin->RenderView(projectionLayerViews[i], swapchainImage, m_colorSwapchainFormat, cubes);

        XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
        CHECK_XRCMD(xrReleaseSwapchainImage(viewSwapchain.handle, &releaseInfo));
    }





    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glViewport(0, 0, w, h);
    glScissor(0, 0, w, h);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image, 0);
	if (depth_supported) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthbuffer, 0);
	} else {
		// TODO: need a depth attachment for depth test when rendering to fbo
	}

	glClearColor(.0f, 0.0f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glUseProgram(shader_program_id);
	glBindVertexArray(VAO);

	int modelLoc = glGetUniformLocation(shader_program_id, "model");
	int colorLoc = glGetUniformLocation(shader_program_id, "uniformColor");
	int viewLoc = glGetUniformLocation(shader_program_id, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float*)viewmatrix.m);
	int projLoc = glGetUniformLocation(shader_program_id, "proj");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, (float*)projectionmatrix.m);


	// render scene with 4 colorful cubes
	{
		// the special color value (0, 0, 0) will get replaced by some UV color in the shader
		glUniform3f(colorLoc, 0.0, 0.0, 0.0);

		double display_time_seconds = ((double)predictedDisplayTime) / (1000. * 1000. * 1000.);
		const float rotations_per_sec = .25;
		float angle = ((long)(display_time_seconds * 360. * rotations_per_sec)) % 360;

		float dist = 1.5f;
		float height = 0.5f;
		render_rotated_cube(vec3(0, height, -dist), .33f, angle, projectionmatrix.m, modelLoc);
		render_rotated_cube(vec3(0, height, dist), .33f, angle, projectionmatrix.m, modelLoc);
		render_rotated_cube(vec3(dist, height, 0), .33f, angle, projectionmatrix.m, modelLoc);
		render_rotated_cube(vec3(-dist, height, 0), .33f, angle, projectionmatrix.m, modelLoc);
	}

	// render controllers
	for (int hand = 0; hand < 2; hand++) {
		if (hand == 0) {
			glUniform3f(colorLoc, 1.0, 0.5, 0.5);
		} else {
			glUniform3f(colorLoc, 0.5, 1.0, 0.5);
		}

		bool hand_location_valid =
		    //(spaceLocation[hand].locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
		    (hand_locations[hand].locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0;

		// draw a block at the controller pose
		if (!hand_location_valid)
			continue;

		XrVector3f scale = {.x = .05f, .y = .05f, .z = .2f};
		render_block(&hand_locations[hand].pose.position, &hand_locations[hand].pose.orientation,
		             &scale, modelLoc);
	}

	// blit left eye to desktop window
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (view_index == 0) {
		_glBlitNamedFramebuffer((GLuint)framebuffer,             // readFramebuffer
		                        (GLuint)0,                       // backbuffer     // drawFramebuffer
		                        (GLint)0,                        // srcX0
		                        (GLint)0,                        // srcY0
		                        (GLint)w,                        // srcX1
		                        (GLint)h,                        // srcY1
		                        (GLint)0,                        // dstX0
		                        (GLint)0,                        // dstY0
		                        (GLint)w / 2,                    // dstX1
		                        (GLint)h / 2,                    // dstY1
		                        (GLbitfield)GL_COLOR_BUFFER_BIT, // mask
		                        (GLenum)GL_LINEAR);              // filter

		SDL_GL_SwapWindow(desktop_window);
	}
#endif

    // XXXX
    return false;
}

void VR::Helper_::CheckXr_(XrResult res, const char *cmd,
                           const char *file, int line) {
    std::cerr << "XXXX <" << cmd << ">\n"; // XXXX
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

void VR::Helper_::Disaster_(const char *msg) {
    std::cerr << "*** " << msg << ": Expect disaster\n";
}
