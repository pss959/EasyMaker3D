#pragma once

#include <vector>

#include "Interfaces/IEmitter.h"
#include "Interfaces/IHandler.h"
#include "Interfaces/IViewer.h"
#include "Math/Types.h"
#include "SG/Typedefs.h"
#include "VR/OpenXRVRBase.h"

class OpenXRVRInput;

//! The OpenXRVR class uses OpenXR to implement the IViewer, IEmitter, and
//! IHandler interfaces.
class OpenXRVR : public OpenXRVRBase,
                 public IViewer, public IEmitter, public IHandler {
  public:
    OpenXRVR();
    virtual ~OpenXRVR();

    virtual const char * GetClassName() const override { return "OpenXRVR"; }

    //! Initializes the viewer to use the given VRCamera, returning false if
    //! anything goes wrong.
    virtual bool Init(const SG::VRCameraPtr &camera);

    // ------------------------------------------------------------------------
    // IViewer interface.
    // ------------------------------------------------------------------------
    virtual void Render(const SG::Scene &scene, IRenderer &renderer);

    // ------------------------------------------------------------------------
    // IEmitter interface.
    // ------------------------------------------------------------------------
    virtual void EmitEvents(std::vector<Event> &events) override;

    // ------------------------------------------------------------------------
    // IHandler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    // TODO: Document All Of This.

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

    typedef std::vector<Swapchain_>                       Swapchains_;
    typedef std::vector<XrViewConfigurationView>          ViewConfigs_;
    typedef std::vector<XrView>                           Views_;
    typedef std::vector<XrCompositionLayerProjectionView> ProjectionViews_;
    typedef std::vector<XrCompositionLayerDepthInfoKHR>   DepthInfos_;

    static constexpr float kZNear = 0.01f;
    static constexpr float kZFar  = 200.0f;

    //! Stores the camera that is used to get the current height offset.
    SG::VRCameraPtr camera_;

    const XrViewConfigurationType view_type_ =
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    int                  fb_ = -1;
    XrInstance           instance_        = nullptr;
    XrSystemId           system_id_       = XR_NULL_SYSTEM_ID;
    XrSession            session_         = XR_NULL_HANDLE;
    XrSessionState       session_state_   = XR_SESSION_STATE_UNKNOWN;
    XrSpace              reference_space_ = XR_NULL_HANDLE;
    XrTime               time_            = 0;
    Swapchains_          swapchains_;
    ViewConfigs_         view_configs_;
    Views_               views_;
    ProjectionViews_     projection_views_;
    DepthInfos_          depth_infos_;
    std::unique_ptr<OpenXRVRInput> input_;

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

    // Helpers.
    void        PrintInstanceProperties_();
    int64_t     GetSwapchainFormat_(int64_t preferred);
    void        InitImages_(Swapchain_::SC_ &sc, uint32_t count);

    void        PollEvents_(std::vector<Event> &events);
    bool        GetNextEvent_(XrEventDataBuffer &event);
    bool        ProcessSessionStateChange_(
        const XrEventDataSessionStateChanged &event);
    void        PollInput_(std::vector<Event> &events);
    void        Render_(const SG::Scene &scene, IRenderer &renderer);
    bool        RenderViews_(const SG::Scene &scene, IRenderer &renderer);
    void        RenderView_(const SG::Scene &scene, IRenderer &renderer,
                            int view_index, int color_index, int depth_index);
};
