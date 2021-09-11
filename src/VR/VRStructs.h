#pragma once

#include <X11/Xlib.h>
#include <GL/glx.h>

#define XR_USE_PLATFORM_XLIB
#define XR_USE_GRAPHICS_API_OPENGL
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

//! OpenXR uses a lot of C structs in its interface. Unfortunately, there are
//! no default constructors for these (because C), so the code that uses them
//! has to fill in every field explicitly. There is an OpenXR-Hpp project that
//! aims to do this automatically, but it does not seem to compile correctly.
//!
//! This namespace provides static factory functions for all of the necessary
//! OpenXR structs, filling them in explicitly as if they had default
//! constructors.
namespace VRS {

XrActionCreateInfo                   BuildActionCreateInfo();
XrActionSetCreateInfo                BuildActionSetCreateInfo();
XrActionSpaceCreateInfo              BuildActionSpaceCreateInfo();
XrActionStateBoolean                 BuildActionStateBoolean();
XrActionStateGetInfo                 BuildActionStateGetInfo();
XrActionsSyncInfo                    BuildActionsSyncInfo();
XrActiveActionSet                    BuildActiveActionSet();
XrApplicationInfo                    BuildApplicationInfo();
XrCompositionLayerDepthInfoKHR       BuildCompositionLayerDepthInfoKHR();
XrCompositionLayerProjection         BuildCompositionLayerProjection();
XrCompositionLayerProjectionView     BuildCompositionLayerProjectionView();
XrFovf                               BuildFovf();
XrFrameBeginInfo                     BuildFrameBeginInfo();
XrFrameEndInfo                       BuildFrameEndInfo();
XrFrameState                         BuildFrameState();
XrFrameWaitInfo                      BuildFrameWaitInfo();
XrGraphicsBindingOpenGLXlibKHR       BuildGraphicsBindingOpenGLXlibKHR();
XrGraphicsRequirementsOpenGLKHR      BuildGraphicsRequirementsOpenGLKHR();
XrInstanceCreateInfo                 BuildInstanceCreateInfo();
XrInstanceProperties                 BuildInstanceProperties();
XrInteractionProfileSuggestedBinding BuildInteractionProfileSuggestedBinding();
XrPosef                              BuildPosef();
XrReferenceSpaceCreateInfo           BuildReferenceSpaceCreateInfo();
XrSessionActionSetsAttachInfo        BuildSessionActionSetsAttachInfo();
XrSessionBeginInfo                   BuildSessionBeginInfo();
XrSessionCreateInfo                  BuildSessionCreateInfo();
XrSpaceLocation                      BuildSpaceLocation();
XrSwapchainCreateInfo                BuildSwapchainCreateInfo();
XrSwapchainImageAcquireInfo          BuildSwapchainImageAcquireInfo();
XrSwapchainImageOpenGLKHR            BuildSwapchainImageOpenGLKHR();
XrSwapchainImageReleaseInfo          BuildSwapchainImageReleaseInfo();
XrSwapchainImageWaitInfo             BuildSwapchainImageWaitInfo();
XrSwapchainSubImage                  BuildSwapchainSubImage();
XrSystemGetInfo                      BuildSystemGetInfo();
XrView                               BuildView();
XrViewConfigurationView              BuildViewConfigurationView();
XrViewLocateInfo                     BuildViewLocateInfo();
XrViewState                          BuildViewState();

}  // namespace VRS
