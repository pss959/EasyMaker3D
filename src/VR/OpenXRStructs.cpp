#include "OpenXRStructs.h"

namespace OpenXRS {

XrActionCreateInfo BuildActionCreateInfo() {
    XrActionCreateInfo s;
    s.type                   = XR_TYPE_ACTION_CREATE_INFO;
    s.next                   = nullptr;
    s.actionName[0]          = '\0';
    s.actionType             = XR_ACTION_TYPE_MAX_ENUM;
    s.countSubactionPaths    = 0;
    s.subactionPaths         = nullptr;
    s.localizedActionName[0] = '\0';
    return s;
}

XrActionSetCreateInfo BuildActionSetCreateInfo() {
    XrActionSetCreateInfo s;
    s.type                      = XR_TYPE_ACTION_SET_CREATE_INFO;
    s.next                      = nullptr;
    s.actionSetName[0]          = '\0';
    s.localizedActionSetName[0] = '\0';
    s.priority                  = 0;
    return s;
}

XrActionSpaceCreateInfo BuildActionSpaceCreateInfo() {
    XrActionSpaceCreateInfo s;
    s.type              = XR_TYPE_ACTION_SPACE_CREATE_INFO;
    s.next              = nullptr;
    s.action            = XR_NULL_HANDLE;
    s.subactionPath     = XR_NULL_PATH;
    s.poseInActionSpace = BuildPosef();;
    return s;
}

XrActionStateBoolean BuildActionStateBoolean() {
    XrActionStateBoolean s;
    s.type                 = XR_TYPE_ACTION_STATE_BOOLEAN;
    s.next                 = nullptr;
    s.currentState         = XR_FALSE;
    s.changedSinceLastSync = XR_FALSE;
    s.lastChangeTime       = 0;
    s.isActive             = XR_FALSE;
    return s;
}

XrActionStateGetInfo BuildActionStateGetInfo() {
    XrActionStateGetInfo s;
    s.type          = XR_TYPE_ACTION_STATE_GET_INFO;
    s.next          = nullptr;
    s.action        = XR_NULL_HANDLE;
    s.subactionPath = XR_NULL_PATH;
    return s;
}

XrActionsSyncInfo BuildActionsSyncInfo() {
    XrActionsSyncInfo s;
    s.type                  = XR_TYPE_ACTIONS_SYNC_INFO;
    s.next                  = nullptr;
    s.countActiveActionSets = 0;
    s.activeActionSets      = nullptr;
    return s;
}

XrActiveActionSet BuildActiveActionSet() {
    XrActiveActionSet s;
    s.actionSet     = XR_NULL_HANDLE;
    s.subactionPath = XR_NULL_PATH;
    return s;
}

XrApplicationInfo BuildApplicationInfo() {
    XrApplicationInfo s;
    s.applicationName[0] = '\0';
    s.applicationVersion = 0;
    s.engineName[0]      = '\0';
    s.engineVersion      = 0;
    s.apiVersion         = 0;
    return s;
}

XrCompositionLayerDepthInfoKHR BuildCompositionLayerDepthInfoKHR() {
    XrCompositionLayerDepthInfoKHR s;
    s.type     = XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR;
    s.next     = nullptr;
    s.subImage = BuildSwapchainSubImage();
    s.minDepth = 0.f;
    s.maxDepth = 0.f;
    s.nearZ    = 0.f;
    s.farZ     = 0.f;
    return s;
}

XrCompositionLayerProjection BuildCompositionLayerProjection() {
    XrCompositionLayerProjection s;
    s.type       = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
    s.next       = nullptr;
    s.layerFlags = 0;
    s.space      = nullptr;
    s.viewCount  = 0;
    s.views      = nullptr;
    return s;
}

XrCompositionLayerProjectionView BuildCompositionLayerProjectionView() {
    XrCompositionLayerProjectionView s;
    s.type     = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
    s.next     = nullptr;
    s.pose     = BuildPosef();
    s.fov      = BuildFovf();
    s.subImage = BuildSwapchainSubImage();
    return s;
}

XrFovf BuildFovf() {
    XrFovf s;
    s.angleLeft  = 0;
    s.angleRight = 0;
    s.angleUp    = 0;
    s.angleDown  = 0;
    return s;
}

XrFrameBeginInfo BuildFrameBeginInfo() {
    XrFrameBeginInfo s;
    s.type = XR_TYPE_FRAME_BEGIN_INFO;
    s.next = nullptr;
    return s;
}

XrFrameEndInfo BuildFrameEndInfo() {
    XrFrameEndInfo s;
    s.type                 = XR_TYPE_FRAME_END_INFO;
    s.next                 = nullptr;
    s.displayTime          = 0;
    s.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    s.layerCount           = 0;
    s.layers               = nullptr;
    return s;
}

XrFrameState BuildFrameState() {
    XrFrameState s;
    s.type                   = XR_TYPE_FRAME_STATE;
    s.next                   = nullptr;
    s.predictedDisplayTime   = 0;
    s.predictedDisplayPeriod = 0;
    s.shouldRender           = XR_FALSE;
    return s;
}

XrFrameWaitInfo BuildFrameWaitInfo() {
    XrFrameWaitInfo s;
    s.type = XR_TYPE_FRAME_WAIT_INFO;
    s.next = nullptr;
    return s;
}

XrGraphicsBindingOpenGLXlibKHR BuildGraphicsBindingOpenGLXlibKHR() {
    XrGraphicsBindingOpenGLXlibKHR s;
    s.type        = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR;
    s.next        = nullptr;
    s.xDisplay    = nullptr;
    s.visualid    = 0;
    s.glxFBConfig = 0;
    s.glxDrawable = 0;
    s.glxContext  = 0;
    return s;
}

XrGraphicsRequirementsOpenGLKHR BuildGraphicsRequirementsOpenGLKHR() {
    XrGraphicsRequirementsOpenGLKHR s;
    s.type                   = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR;
    s.next                   = nullptr;
    s.minApiVersionSupported = 0;
    s.maxApiVersionSupported = 0;
    return s;
}

XrInstanceCreateInfo BuildInstanceCreateInfo() {
    XrInstanceCreateInfo s;
    s.type                  = XR_TYPE_INSTANCE_CREATE_INFO;
    s.next                  = nullptr;
    s.createFlags           = 0;
    s.applicationInfo       = BuildApplicationInfo();
    s.enabledApiLayerCount  = 0;
    s.enabledApiLayerNames  = nullptr;
    s.enabledExtensionCount = 0;
    s.enabledExtensionNames = nullptr;
    return s;
}

XrInstanceProperties BuildInstanceProperties() {
    XrInstanceProperties s;
    s.type           = XR_TYPE_INSTANCE_PROPERTIES;
    s.next           = nullptr;
    s.runtimeVersion = 0;
    s.runtimeName[0] = '\0';
    return s;
}

XrInteractionProfileSuggestedBinding BuildInteractionProfileSuggestedBinding() {
    XrInteractionProfileSuggestedBinding s;
    s.type                   = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
    s.next                   = nullptr;
    s.interactionProfile     = XR_NULL_PATH;
    s.countSuggestedBindings = 0;
    s.suggestedBindings      = nullptr;
    return s;
}

XrPosef BuildPosef() {
    XrPosef s;
    s.orientation.x = 0;
    s.orientation.y = 0;
    s.orientation.z = 0;
    s.orientation.w = 1;
    s.position.x    = 0;
    s.position.y    = 0;
    s.position.z    = 0;
    return s;
}

XrReferenceSpaceCreateInfo BuildReferenceSpaceCreateInfo() {
    XrReferenceSpaceCreateInfo s;
    s.type                 = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
    s.next                 = nullptr;
    s.referenceSpaceType   = XR_REFERENCE_SPACE_TYPE_VIEW;
    s.poseInReferenceSpace = BuildPosef();
    return s;
}

XrSessionActionSetsAttachInfo BuildSessionActionSetsAttachInfo() {
    XrSessionActionSetsAttachInfo s;
    s.type            = XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO;
    s.next            = nullptr;
    s.countActionSets = 0;
    s.actionSets      = nullptr;
    return s;
}

XrSessionBeginInfo BuildSessionBeginInfo() {
    XrSessionBeginInfo s;
    s.type                         = XR_TYPE_SESSION_BEGIN_INFO;
    s.next                         = nullptr;
    s.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
    return s;
}

XrSessionCreateInfo BuildSessionCreateInfo() {
    XrSessionCreateInfo s;
    s.type        = XR_TYPE_SESSION_CREATE_INFO;
    s.next        = nullptr;
    s.createFlags = 0;
    s.systemId    = 0;
    return s;
}

XrSpaceLocation BuildSpaceLocation() {
    XrSpaceLocation s;
    s.type          = XR_TYPE_SPACE_LOCATION;
    s.next          = nullptr;
    s.locationFlags = 0;
    s.pose          = BuildPosef();
    return s;
}

XrSwapchainCreateInfo BuildSwapchainCreateInfo() {
    XrSwapchainCreateInfo s;
    s.type        = XR_TYPE_SWAPCHAIN_CREATE_INFO;
    s.next        = nullptr;
    s.createFlags = 0;
    s.usageFlags  = 0;
    s.format      = 0;
    s.sampleCount = 0;
    s.width       = 0;
    s.height      = 0;
    s.faceCount   = 0;
    s.arraySize   = 0;
    s.mipCount    = 0;
    return s;
}

XrSwapchainImageAcquireInfo BuildSwapchainImageAcquireInfo() {
    XrSwapchainImageAcquireInfo s;
    s.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
    s.next = nullptr;
    return s;
}

XrSwapchainImageOpenGLKHR BuildSwapchainImageOpenGLKHR() {
    XrSwapchainImageOpenGLKHR s;
    s.type  = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
    s.next  = nullptr;
    s.image = 0;
    return s;
}

XrSwapchainImageReleaseInfo BuildSwapchainImageReleaseInfo() {
    XrSwapchainImageReleaseInfo s;
    s.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
    s.next = nullptr;
    return s;
}

XrSwapchainImageWaitInfo BuildSwapchainImageWaitInfo() {
    XrSwapchainImageWaitInfo s;
    s.type    = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
    s.next    = nullptr;
    s.timeout = 0;
    return s;
}

XrSwapchainSubImage BuildSwapchainSubImage() {
    XrSwapchainSubImage s;
    s.swapchain               = XR_NULL_HANDLE;
    s.imageRect.offset.x      = 0;
    s.imageRect.offset.y      = 0;
    s.imageRect.extent.width  = 0;
    s.imageRect.extent.height = 0;
    s.imageArrayIndex         = 0;
    return s;
}

XrSystemGetInfo BuildSystemGetInfo() {
    XrSystemGetInfo s;
    s.type       = XR_TYPE_SYSTEM_GET_INFO;
    s.next       = nullptr;
    s.formFactor = XR_FORM_FACTOR_MAX_ENUM;
    return s;
}

XrView BuildView() {
    XrView s;
    s.type = XR_TYPE_VIEW;
    s.next = nullptr;
    s.pose = BuildPosef();
    s.fov  = BuildFovf();
    return s;
}

XrViewConfigurationView BuildViewConfigurationView() {
    XrViewConfigurationView s;
    s.type                            = XR_TYPE_VIEW_CONFIGURATION_VIEW;
    s.next                            = nullptr;
    s.recommendedImageRectWidth       = 0;
    s.maxImageRectWidth               = 0;
    s.recommendedImageRectHeight      = 0;
    s.maxImageRectHeight              = 0;
    s.recommendedSwapchainSampleCount = 0;
    s.maxSwapchainSampleCount         = 0;
    return s;
}

XrViewLocateInfo BuildViewLocateInfo() {
    XrViewLocateInfo s;
    s.type                  = XR_TYPE_VIEW_LOCATE_INFO;
    s.next                  = nullptr;
    s.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
    s.displayTime           = 0;
    s.space                 = XR_NULL_HANDLE;
    return s;
}

XrViewState BuildViewState() {
    XrViewState s;
    s.type           = XR_TYPE_VIEW_STATE;
    s.next           = nullptr;
    s.viewStateFlags = 0;
    return s;
}

}  // namespace OpenXRS
