#pragma once

#include <GLFW/glfw3.h>

// Allow access to native types in glfw3.
#if defined(ION_PLATFORM_LINUX)
#  define GLFW_EXPOSE_NATIVE_GLX
#  define GLFW_EXPOSE_NATIVE_X11
#elif defined(ION_PLATFORM_WINDOWS)
#  define GLFW_EXPOSE_NATIVE_WGL
#endif
#include <GLFW/glfw3native.h>

/// The ViewerContext struct encapsulates platform-dependent data that is
/// needed to set up VR. It allows this information to be accessed from the
/// GLFWViewer and passed to the VRViewer.
struct ViewerContext {
#if defined(ION_PLATFORM_LINUX)
    Display     *display;   ///< X11 display.
    GLXContext   context;   ///< GLX context.
    GLXDrawable  drawable;  ///< GLX drawable.

#elif defined(ION_PLATFORM_WINDOWS)
    HDC          dc;        ///< Device context.
    HGLRC        glrc;      ///< GL context.

#endif
};
