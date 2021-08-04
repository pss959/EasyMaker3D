#pragma once

#include <X11/Xlib.h>
#include <GL/glx.h>

#define XR_USE_PLATFORM_XLIB
#define XR_USE_GRAPHICS_API_OPENGL
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <string>
#include <vector>

#include "Interfaces/IEmitter.h"
#include "Interfaces/IHandler.h"
#include "Interfaces/IViewer.h"

//! GLFWViewer uses the GLFW library to implement the IViewer and IEmitter, and
//! IHandler interfaces.
class OpenXRVR : public IViewer, public IEmitter, public IHandler {
  public:
    OpenXRVR();
    virtual ~OpenXRVR();

    virtual const char * GetClassName() const override { return "OpenXRVR"; }
    // ------------------------------------------------------------------------
    // IViewer interface.
    // ------------------------------------------------------------------------
    virtual bool Init(const ion::math::Vector2i &size);
    virtual void SetSize(const ion::math::Vector2i &new_size) override;
    virtual ion::math::Vector2i GetSize() const override;
    virtual void Render(IScene &scene, IRenderer &renderer) override;

    // ------------------------------------------------------------------------
    // IEmitter interface.
    // ------------------------------------------------------------------------
    virtual void EmitEvents(std::vector<Event> &events) override;

    // ------------------------------------------------------------------------
    // IHandler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    // XXXX Document All Of This.

    //! Exception thrown when any function fails.
    class VRException_ : public std::exception {
      public:
        VRException_(const std::string &msg) : msg_(msg) {}
        const char * what() const throw() override { return msg_.c_str(); }
      private:
        std::string msg_;
    };

    //! Stores information for each XrSwapchain.
    struct Swapchain_ {
        struct SC_ {
            XrSwapchain                               swapchain;
            std::vector<XrSwapchainImageOpenGLKHR>    gl_images;
            std::vector<XrSwapchainImageBaseHeader *> images;
        };
        SC_ color;
        SC_ depth;
    };

    //! Stores the current state of all inputs.
    struct InputState_ {
        //! Stores the input state for each controller.
        struct ControllerState_ {
            XrPath   path;
            XrSpace  space;
            float    scale = 1.0f;
            XrBool32 active;
        };

        XrActionSet      action_set   = XR_NULL_HANDLE;
        XrAction         pinch_action = XR_NULL_HANDLE;
        XrAction         grip_action  = XR_NULL_HANDLE;
        XrAction         menu_action  = XR_NULL_HANDLE;

        ControllerState_ controller_state[2];
    };

    //! This struct represents the binding of an input (by path string) to an
    //! action in the InputState_.
    struct InputBinding_ {
        XrAction    action;
        std::string path_name;
        XrPath      path;
    };

    typedef std::vector<Swapchain_>                       Swapchains_;
    typedef std::vector<XrViewConfigurationView>          ViewConfigs_;
    typedef std::vector<XrView>                           Views_;
    typedef std::vector<XrCompositionLayerProjectionView> ProjectionViews_;
    typedef std::vector<XrCompositionLayerDepthInfoKHR>   DepthInfos_;

    static constexpr float kZNear = 0.01f;
    static constexpr float kZFar  = 100.0f;

    const XrViewConfigurationType view_type_ =
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    int                  fb_ = -1;
    XrInstance           instance_        = nullptr;
    XrSystemId           system_id_       = XR_NULL_SYSTEM_ID;
    XrSession            session_         = XR_NULL_HANDLE;
    XrSessionState       session_state_   = XR_SESSION_STATE_UNKNOWN;
    XrSpace              reference_space_ = XR_NULL_HANDLE;
    Swapchains_          swapchains_;
    ViewConfigs_         view_configs_;
    Views_               views_;
    ProjectionViews_     projection_views_;
    DepthInfos_          depth_infos_;
    InputState_          input_state_;

    // Initialization subfunctions.
    bool InitInstance_();
    void InitSystem_();
    void InitViewConfigs_();
    void InitRendering_(IRenderer &renderer);
    void InitViews_();
    void InitSession_(IRenderer &renderer);
    void InitReferenceSpace_();
    void InitSwapchains_();
    void InitProjectionViews_();
    void InitInput_();

    // Helpers.
    void        PrintInstanceProperties_();
    int64_t     GetSwapchainFormat_(int64_t preferred);
    void        InitImages_(Swapchain_::SC_ &sc, uint32_t count);
    void        CreateInputAction_(const char *name, XrAction &action);
    void        AddControllerBindings();
    InputBinding_ BuildInputBinding_(const std::string &path_name,
                                     XrAction action);

    void        PollEvents_(std::vector<Event> &events);
    bool        GetNextEvent_(XrEventDataBuffer &event);
    bool        ProcessSessionStateChange_(
        const XrEventDataSessionStateChanged &event);
    void        PollInput_(std::vector<Event> &events);
    void        RenderScene_(IScene &scene, IRenderer &renderer);
    bool        RenderViews_(IScene &scene, IRenderer &renderer,
                             XrTime predicted_display_time);
    void        RenderView_(IScene &scene, IRenderer &renderer,
                            int view_index, int color_index, int depth_index);

    // Error checking and reporting.
    void CheckXr_(XrResult res, const char *cmd, const char *file, int line);
    void Assert_(bool exp, const char *expstr, const char *file, int line);
    void Throw_(const std::string &msg);
    void ReportException_(const VRException_ &ex);
    void ReportDisaster_(const char *msg);
};
