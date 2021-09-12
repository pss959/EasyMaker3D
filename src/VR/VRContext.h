#pragma once

#include <vector>

#include "Math/Types.h"
#include "SG/Typedefs.h"
#include "VR/VRBase.h"

class Renderer;
struct Event;

//! VRContext defines a environment that sets up OpenXR for use in the
//! application.
class VRContext : public VRBase {
  public:
    VRContext();
    ~VRContext();

    //! Initializes the context. Returns false if anything goes wrong.
    bool Init();

    //! Initializes rendering. This must be called once before Render().
    void InitRendering(Renderer &renderer);

    //! Renders.
    void Render(const SG::Scene &scene, Renderer &renderer,
                const Point3f &base_position);

    //! Returns the XrInstance.
    XrInstance GetInstance() const { return instance_; }

    //! Returns the XrSession.
    XrSession  GetSession()  const { return session_; }

    //! Returns the reference space.
    XrSpace    GetReferenceSpace() const { return reference_space_; }

    //! Returns the current time.
    XrTime     GetTime() const { return time_; }

    //! Returns the view type.
    XrViewConfigurationType GetViewType() const { return view_type_; }

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

    static constexpr float kZNear = .6f;  // Anything smaller causes z-fighting.
    static constexpr float kZFar  = 260.0f;

    const XrViewConfigurationType view_type_ =
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    int                  fb_ = -1;
    XrInstance           instance_        = nullptr;
    XrSystemId           system_id_       = XR_NULL_SYSTEM_ID;
    XrSession            session_         = XR_NULL_HANDLE;
    XrSpace              reference_space_ = XR_NULL_HANDLE;
    XrTime               time_            = 0;
    Swapchains_          swapchains_;
    ViewConfigs_         view_configs_;
    Views_               views_;
    ProjectionViews_     projection_views_;
    DepthInfos_          depth_infos_;

    // Initialization subfunctions.
    bool InitInstance_();
    void InitSystem_();
    void InitViewConfigs_();
    void InitRendering_(Renderer &renderer);
    void InitViews_();
    void InitSession_(Renderer &renderer);
    void InitReferenceSpace_();
    void InitSwapchains_();
    void InitProjectionViews_();

    // Helpers.
    void    PrintInstanceProperties_();
    int64_t GetSwapchainFormat_(int64_t preferred);
    void    InitImages_(Swapchain_::SC_ &sc, uint32_t count);
    void    Render_(const SG::Scene &scene, Renderer &renderer,
                    const Point3f &base_position);
    bool    RenderViews_(const SG::Scene &scene, Renderer &renderer,
                         const Point3f &base_position);
    void    RenderView_(const SG::Scene &scene, Renderer &renderer,
                        const Point3f &base_position, int view_index,
                        int color_index, int depth_index);
};
