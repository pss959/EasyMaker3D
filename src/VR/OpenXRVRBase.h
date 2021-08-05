#pragma once

#include <X11/Xlib.h>
#include <GL/glx.h>

#define XR_USE_PLATFORM_XLIB
#define XR_USE_GRAPHICS_API_OPENGL
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <exception>
#include <string>

//! Abstract base class for OpenXRVR classes, containing utilities.
class OpenXRVRBase {
  protected:
    //! Exception thrown when any function fails.
    class VRException_ : public std::exception {
      public:
        VRException_(const std::string &msg) : msg_(msg) {}
        const char * what() const throw() override { return msg_.c_str(); }
      private:
        std::string msg_;
    };

    //! Sets the XrInstance to use for error checking and reporting.
    void SetInstance(XrInstance instance) {
        instance_ = instance;
    }

    // Error checking and reporting.
    void CheckXr_(XrResult res, const char *cmd, const char *file, int line);
    void Assert_(bool exp, const char *expstr, const char *file, int line);
    void Throw_(const std::string &msg);
    void ReportException_(const VRException_ &ex);
    void ReportDisaster_(const char *msg);

  private:
    XrInstance instance_ = XR_NULL_HANDLE;
};

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

